#include <kernel/memory.h>

static uint64_t *current_pml4;
static uint64_t offset;

int8_t page_mapper_init(uint64_t memory_offset, uint64_t *pml4) {
    current_pml4 = pml4;
    offset = memory_offset;
    return 0;
}

int8_t page_map_in_table(void *pml4, virtual_addr va_addr, physical_addr phys_addr) {
    uint64_t temp = current_pml4;
    current_pml4 = pml4;
    int8_t result = page_map(va_addr, phys_addr);
    current_pml4 = temp;
    return result;
}

int8_t page_map(virtual_addr va_addr, physical_addr phys_addr) {
    uint64_t pml4_index = VA_GET_PML4_INDEX(va_addr);
    uint64_t pdp_index  = VA_GET_PDP_INDEX(va_addr);
    uint64_t pd_index   = VA_GET_PD_INDEX(va_addr);
    uint64_t pt_index   = VA_GET_PT_INDEX(va_addr);

    uint64_t *pml4_pointer = (uint64_t)current_pml4 + offset;
    if (!(pml4_pointer[pml4_index] & PRESENT_FLAG)) {
        // @TODO: Allocate a page to do this.
        uint64_t *next_page;
        if (page_physical_alloc(&next_page) == -1) {
            return -1;
        }
        for (uint64_t i = 0; i < 0x200; i++) {
            next_page[i] = 0;
        }
        pml4_pointer[pml4_index] = PRESENT_FLAG | WRITABLE_FLAG | USER_ACCESS_FLAG;
        pml4_pointer[pml4_index] |= (uint64_t)next_page;
    }

    uint64_t *pdp_pointer = (uint64_t)(pml4_pointer[pml4_index] & ADDR_MASK) + offset;
    if (!(pdp_pointer[pdp_index] & PRESENT_FLAG)) {
        // @TODO: Allocate a page to do this.
        uint64_t *next_page;
        if (page_physical_alloc(&next_page) == -1) {
            return -1;
        }
        for (uint64_t i = 0; i < 0x200; i++) {
            next_page[i] = 0;
        }
        pdp_pointer[pdp_index] = PRESENT_FLAG | WRITABLE_FLAG | USER_ACCESS_FLAG;
        pdp_pointer[pdp_index] |= (uint64_t)next_page;
    }

    uint64_t *pd_pointer = (uint64_t)(pdp_pointer[pdp_index] & ADDR_MASK) + offset;
    if (!(pd_pointer[pd_index] & PRESENT_FLAG)) {
        // @TODO: Allocate a page to do this.
        uint64_t *next_page;
        if (page_physical_alloc(&next_page) == -1) {
            return -1;
        }
        for (uint64_t i = 0; i < 0x200; i++) {
            next_page[i] = 0;
        }
        pd_pointer[pd_index] = PRESENT_FLAG | WRITABLE_FLAG | USER_ACCESS_FLAG;
        pd_pointer[pd_index] |= (uint64_t)next_page;
    }

    uint64_t *pt_pointer = (uint64_t)(pd_pointer[pd_index] & ADDR_MASK) + offset;
    if (!(pt_pointer[pt_index] & PRESENT_FLAG)) {
        pt_pointer[pt_index] = PRESENT_FLAG | WRITABLE_FLAG | USER_ACCESS_FLAG;
        pt_pointer[pt_index] |= phys_addr & ADDR_MASK;
    } else {
        return 1;
    }
    return 0;
}

int64_t page_map_multiple(uint64_t count, virtual_addr va_addr, physical_addr *phys_addr) {
    for (uint64_t index = 0; index < count; index++) {
        if (page_map(va_addr + index << 12, phys_addr[index])) {
            return index + 1;
        }
    }
    return 0;
}

int8_t page_map_set(void *pml4_pointer) {
    // @TEMPORARY: This works only if the map is identity mapped in all maps
    current_pml4 = pml4_pointer;
    uint64_t cr3;
    __asm__ __volatile__ (
        "movq %%cr3, %0;" : "=r" (cr3) :
    );
    // Clear address
    cr3 &= ~ADDR_MASK;
    // Set address
    cr3 |= (uint64_t)pml4_pointer & ADDR_MASK;
    __asm__ __volatile__ (
        "movq %0, %%cr3;" : : "r" (cr3)
    );

    // @TODO: Confirm that this procedure actually worked?
    return 0;
}