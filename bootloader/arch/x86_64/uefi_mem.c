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

void get_mapping_ptrs_and_count(uint64_t *pml4_ptr_in, uint64_t *next_available_mapping_page_in, uint64_t *available_mapping_page_count_in) {
    *pml4_ptr_in = pml4_ptr;
    *next_available_mapping_page_in = next_available_mapping_page;
    *available_mapping_page_count_in = available_mapping_page_count;
}
