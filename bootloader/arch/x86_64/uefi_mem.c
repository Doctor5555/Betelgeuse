#include "uefi_mem.h"

#include "uefi_tty.h"

static uint64_t *pml4_ptr;
/* Absolute physical address of the next page available to map */
static physical_addr next_available_mapping_page;
static uint64_t available_mapping_page_count;

uint64_t get_next_available_mapping_page() {
    next_available_mapping_page += 0x1000;
    available_mapping_page_count--;
    return next_available_mapping_page - 0x1000;
}

/* Borrowed and translated from https://os.phil-opp.com/page-tables/, along with some other code */
uint64_t next_table_address(uint64_t *current, uint64_t index) {
    if (current[index] & PRESENT_FLAG && !(current[index] & PAGE_SIZE_FLAG)) {
        return ((uint64_t)current << 9) | (index << 12);
    } else {
        return 0;
    }
}

efi_status init_virtual_mapping(efi_memory_descriptor *mem_map, uint64_t descriptor_count, uint64_t descriptor_size) {
    /* CR3 holds the address of the highest level page mapping table */
    uint64_t cr3;
    __asm__ __volatile__ (
        "movq %%cr3, %0;" : "=r" (cr3) :
    );
    uint64_t pml4_addr = cr3 & ADDR_MASK;

    /* Find the index and address of the mapping tables */
    uint64_t pml4_index = 0;
    uint64_t pml4_page_count = 0;
    uint64_t src_addr = 0;
    for (uint64_t i = 0; i < descriptor_count; i++) {
        efi_memory_descriptor *descriptor = (uint64_t)mem_map + i * descriptor_size;
        if (descriptor->PhysicalStart > cr3) {
            descriptor = (uint64_t)mem_map + (i - 1) * descriptor_size;
            pml4_index = i - 1;
            src_addr = descriptor->PhysicalStart;
            pml4_page_count = descriptor->NumberOfPages;
            break;
        }
        if (i == descriptor_count - 1) {
            return EFI_NOT_FOUND;
        }
    }

    /* Find a valid free location to move the mapping tables to */
    uint64_t map_size = pml4_page_count << 12;
    uint64_t dest_addr = 0;
    uint64_t dest_index = 0;
    uint8_t *new_map = 0;
    uint8_t *old_map = src_addr;
    for (uint64_t i = 0; i < descriptor_count; i++) {
        efi_memory_descriptor *descriptor = (uint64_t)mem_map + i * descriptor_size;
        if ((descriptor->NumberOfPages >= pml4_page_count) &&
            (descriptor->Type == 0x7) && 
            !(descriptor->Attribute & 0x8000000000000000)) {
            dest_index = i;
            break;
        }
        if (i == descriptor_count - 1) {
            return EFI_BUFFER_TOO_SMALL;
        }
    }

    /* Get the destination address and copy the memory from the original tables to the new tables */
    efi_memory_descriptor *dest_desc = (uint64_t)mem_map + dest_index * descriptor_size;
    dest_addr = dest_desc->PhysicalStart;

    new_map = (uint8_t *)dest_addr;
    for (uint64_t i = 0; i < map_size; i++) {
        new_map[i] = old_map[i];
    }

    /* Locate the first available page for new mappings, and change addresses for the new location */
    #define old_addr_to_new_map(x) ((uint64_t)new_map + ((x) - (uint64_t)old_map))
    uint64_t *pml4 = (uint64_t)new_map + cr3 - (uint64_t)old_map;
    uint64_t highest_addr = 0;
    for (int i = 0; i < 512; i++) {
        if (pml4[i] & PRESENT_FLAG) {
            if ((uint64_t)&pml4[i] > highest_addr) {
                highest_addr = (uint64_t)&pml4[i];
            }
            uint64_t temp_addr = old_addr_to_new_map(pml4[i] & ADDR_MASK);
            pml4[i] &= ~ADDR_MASK;
            pml4[i] |= temp_addr;
            uint64_t *pdp = pml4[i] & ADDR_MASK;
            for (int j = 0; j < 512; j++) {
                if (pdp[j] & PRESENT_FLAG) {
                    if ((uint64_t)&pdp[j] > highest_addr) {
                        highest_addr = (uint64_t)&pdp[j];
                    }
                    if (!(pdp[j] & PAGE_SIZE_FLAG)) {
                        uint64_t temp_addr = old_addr_to_new_map(pdp[j] & ADDR_MASK);
                        pdp[j] &= ~ADDR_MASK;
                        pdp[j] |= temp_addr;
                        uint64_t *pd = pdp[j] & ADDR_MASK;
                        for (int k = 0; k < 512; k++) {
                            if ((pd[k] & PRESENT_FLAG) && (uint64_t)&pd[k] > highest_addr) {
                                highest_addr = (uint64_t)&pd[k];
                            }
                        }
                    } else {
                        terminal_writestring("PDP entry with PS bit set!\n\r");
                    }
                }
            }
        }
    }
    #undef old_addr_to_new_map

    next_available_mapping_page = 0x1000 + highest_addr - (highest_addr % 0x1000);
    available_mapping_page_count = ((uint64_t)new_map + (pml4_page_count << 12) - next_available_mapping_page) >> 12;

    /* Set up recursive mapping */
    //pml4[511] = PRESENT_FLAG | WRITABLE_FLAG | (uint64_t)pml4;

    /* Save the address of the old pml4, and put the new mapping tables in CR3 */
    struct PML4_entry *old_pml4 = (struct PML4_entry *)cr3;
    pml4_ptr = (uint64_t)new_map + pml4_addr - (uint64_t)old_map;
    cr3 &= ~ADDR_MASK;
    cr3 |= ((uint64_t)new_map + pml4_addr - (uint64_t)old_map) & ADDR_MASK;
    __asm__ __volatile__ (
        "movq %0, %%cr3;" : : "r" (cr3)
    );

    /* Set the old page tables to R/W */
    /*struct PML4_entry *pml4_pml4 = (struct PML4_entry *)cr3 + ((uint64_t)(PML4_MASK(old_pml4)) >> PML4_OFFSET);
    struct PDP_entry *pml4_pdp = (struct PDP_entry*)(uint64_t)(pml4_pml4->addr << 12) + ((uint64_t)(PDP_MASK(old_pml4)) >> PDP_OFFSET);
    struct PD_entry_2M *pml4_pd = (struct PD_entry_2M*)(uint64_t)(pml4_pdp->addr << 12) + ((uint64_t)(PD_MASK(old_pml4)) >> PD_OFFSET);
    pml4_pd->R_W = 1;*/

    return EFI_SUCCESS;
}

efi_status map_pages(physical_addr src_addr, virtual_addr dest_addr, uint64_t page_count) {
    for (uint64_t i = 0; i < page_count; i++) {
        map_page(src_addr + (i << 12), dest_addr + (i << 12));
    }
}

efi_status map_page(physical_addr phys_addr, virtual_addr va_addr) {
    int pml4_index = VA_GET_PML4_INDEX(va_addr);
    int pdp_index  = VA_GET_PDP_INDEX(va_addr);
    int pd_index   = VA_GET_PD_INDEX(va_addr);
    int pt_index   = VA_GET_PT_INDEX(va_addr);
    
    if (!(pml4_ptr[pml4_index] & PRESENT_FLAG)) {
        terminal_writestring("Writing PML4: 0x");
        uint64_t *next_page = get_next_available_mapping_page();
        for (uint64_t i = 0; i < 0x200; i++) {
            next_page[i] = 0;
        }
        pml4_ptr[pml4_index] = PRESENT_FLAG | WRITABLE_FLAG;
        pml4_ptr[pml4_index] |= (uint64_t)next_page;
        terminal_print_hex64(pml4_ptr[pml4_index]);
        terminal_writestring("\n\r");
    } else {
        terminal_writestring("PML4 present: 0x");
        terminal_print_hex64(pml4_ptr[pml4_index]);
        terminal_writestring("\n\r");
    }

    uint64_t *pdp_ptr = pml4_ptr[pml4_index] & ADDR_MASK;
    if (!(pdp_ptr[pdp_index] & PRESENT_FLAG)) {
        uint64_t *next_page = get_next_available_mapping_page();
        for (uint64_t i = 0; i < 0x200; i++) {
            next_page[i] = 0;
        }
        pdp_ptr[pdp_index] = PRESENT_FLAG | WRITABLE_FLAG | USER_ACCESS_FLAG;
        pdp_ptr[pdp_index] |= (uint64_t)next_page;
    }

    uint64_t *pd_ptr = pdp_ptr[pdp_index] & ADDR_MASK;
    if (!(pd_ptr[pd_index] & PRESENT_FLAG)) {
        uint64_t *next_page = get_next_available_mapping_page();
        for (uint64_t i = 0; i < 0x200; i++) {
            next_page[i] = 0;
        }
        pd_ptr[pd_index] = PRESENT_FLAG | WRITABLE_FLAG | USER_ACCESS_FLAG;
        pd_ptr[pd_index] |= (uint64_t)next_page;
    }

    uint64_t *pt_ptr = pd_ptr[pd_index] & ADDR_MASK;
    if (!(pt_ptr[pt_index] & PRESENT_FLAG)) {
        pt_ptr[pt_index] = PRESENT_FLAG | WRITABLE_FLAG | USER_ACCESS_FLAG;
        pt_ptr[pt_index] |= phys_addr & ADDR_MASK;
    } else {
        /* A page we are trying to map is already mapped! This is not allowed. */
        return EFI_INVALID_PARAMETER;
    }

    return EFI_SUCCESS;
}

void get_mapping_ptrs_and_count(uint64_t *next_available_mapping_page_in, uint64_t *available_mapping_page_count_in) {
    next_available_mapping_page_in = next_available_mapping_page;
    available_mapping_page_count_in = available_mapping_page_count;
}

/* I want to use this sort of algorithm to map pages, but it does not currently work
terminal_writestring("src addr: 0x");
    terminal_print_hex64(src_addr);
    terminal_writestring(", dest addr: 0x");
    terminal_print_hex64(dest_addr);
    terminal_writestring("\n\r");
    struct PML4_entry *pml4;
    struct PDP_entry *pml3;
    struct PD_entry_4K *pml2;
    int64_t pml2_decrement = -0x200;
    int64_t pml3_decrement = -0x200;
    int64_t pml4_decrement = -0x200;
    uint64_t pml4_index = ((uint64_t)(PML4_MASK(dest_addr)) >> PML4_OFFSET);
    uint64_t pml3_index = ((uint64_t)(PDP_MASK(dest_addr)) >> PDP_OFFSET);
    uint64_t pml2_index = ((uint64_t)(PD_MASK(dest_addr)) >> PD_OFFSET);
    uint64_t pml1_base_index = ((uint64_t)(PT_MASK(dest_addr)) >> PT_OFFSET);
    for (uint64_t i = 0; i < page_count; i++) {
        if (pml1_base_index + i - pml2_decrement >= 0x200) {
            if (pml2_index - pml3_decrement >= 0x200) {
                if (pml3_index - pml4_decrement >= 0x200) {
                    if (pml4_index >= 0x200) {
                        /* @TODO: When PML5 is available, use PML5 tables here */
                        /* Currently an error as this means we are mapping pages outside the address range *
                        return EFI_BUFFER_TOO_SMALL;
                    }
                    pml4_decrement = pml3_index;
                    pml4 = pml4_ptr + pml4_index;
                    pml4_index++;
                    terminal_writestring("pml4 entry: 0x");
                    terminal_print_hex64(pml4_index);
                    terminal_writestring(", addr: 0x");
                    terminal_print_hex64((uint64_t)pml4);
                    terminal_writestring("\n\r");
                    if (!pml4->P) {
                        uint8_t *pdp_page = next_available_mapping_page;
                        next_available_mapping_page += 0x1000;
                        available_mapping_page_count--;
                        for (uint64_t i = 0; i < 0x1000; i++) {
                            pdp_page[i] = 0;
                        }
                        pml4->R_W = 1;
                        pml4->U_S = 0;
                        pml4->PWT = 0;
                        pml4->PCD = 0;
                        pml4->A = 0;
                        pml4->MBZ = 0;
                        pml4->AVL = 0;
                        pml4->addr = ((uint64_t)pdp_page) >> 12;
                        pml4->available = 0;
                        pml4->NX = 0;
                        pml4->P = 1;
                    }
                }
                pml3_decrement = pml2_index;
                pml3 = (struct PDP_entry*)(uint64_t)(pml4->addr << 12) + pml3_index - pml4_decrement;
                pml3_index++;
                terminal_writestring("pml3 entry: 0x");
                terminal_print_hex64(pml3_index);
                terminal_writestring(", addr: 0x");
                terminal_print_hex64((uint64_t)pml3);
                terminal_writestring("\n\r");
                if (!pml3->P) {
                    uint8_t *pd_page = next_available_mapping_page;
                    next_available_mapping_page += 0x1000;
                    available_mapping_page_count--;
                    for (uint64_t i = 0; i < 0x1000; i++) {
                        pd_page[i] = 0;
                    }
                    pml3->R_W = 1;
                    pml3->U_S = 0;
                    pml3->PWT = 0;
                    pml3->PCD = 0;
                    pml3->A = 0;
                    pml3->PS = 0;
                    pml3->AVL = 0;
                    pml3->addr = ((uint64_t)pd_page) >> 12;
                    pml3->available = 0;
                    pml3->NX = 0;
                    pml3->P = 1;
                }
            }
            pml2_decrement = ((uint64_t)(PT_MASK(dest_addr)) >> PT_OFFSET) + i;
            pml2 = (struct PD_entry_4K*)(uint64_t)(pml3->addr << 12) + pml2_index - pml3_decrement;
            pml2_index++;
            terminal_writestring("pml2 entry: 0x");
            terminal_print_hex64(pml2_index);
            terminal_writestring(", addr: 0x");
            terminal_print_hex64((uint64_t)pml2);
            terminal_writestring("\n\r");
            if (!pml2->P) {
                uint8_t *pt_page = next_available_mapping_page;
                next_available_mapping_page += 0x1000;
                available_mapping_page_count--;
                for (uint64_t i = 0; i < 0x1000; i++) {
                    pt_page[i] = 0;
                }
                pml2->R_W = 1;
                pml2->U_S = 0;
                pml2->PWT = 0;
                pml2->PCD = 0;
                pml2->A = 0;
                pml2->PS = 0;
                pml2->AVL = 0;
                pml2->addr = ((uint64_t)pt_page) >> 12;
                pml2->available = 0;
                pml2->NX = 0;
                pml2->P = 1;
            }
        }
        struct PT_entry *pml1 = (struct PT_entry*)(uint64_t)(pml2->addr << 12) + pml1_base_index + i - pml2_decrement;
        terminal_writestring("pml1 entry: 0x");
        terminal_print_hex64(i);
        terminal_writestring(", addr: 0x");
        terminal_print_hex64((uint64_t)pml1);
        terminal_writestring("\n\r");
        if (!pml1->P) {
            pml1->R_W = 1;
            pml1->U_S = 0;
            pml1->PWT = 0;
            pml1->PCD = 0;
            pml1->A = 0;
            pml1->D = 0;
            pml1->PAT = 0;
            pml1->G = 0;
            pml1->AVL = 0;
            pml1->addr = ((uint64_t)src_addr) >> 12;
            pml1->available = 0;
            pml1->CR4_PKE_dep = 0;
            pml1->NX = 0;
            pml1->P = 1;
        } else {
            /* A page we are trying to map is already mapped! This is not allowed. *
            return EFI_INVALID_PARAMETER;
        }
    }
    return EFI_SUCCESS;
    */