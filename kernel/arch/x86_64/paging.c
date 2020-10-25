#include <kernel/paging.h>

#include <kernel/panic.h>

static uint64_t *pml4_ptr = 0xFFFFFFFFFFFFF000;

static physical_addr next_available_mapping_page;
static uint64_t available_mapping_page_count;

uint64_t get_next_available_mapping_page() {
    next_available_mapping_page += 0x1000;
    available_mapping_page_count--;
    return next_available_mapping_page - 0x1000;
}

void page_map(virtual_addr va_addr, physical_addr phys_addr) {
    uint64_t pml4_index = VA_GET_PML4_INDEX(va_addr);
    uint64_t pdp_index  = VA_GET_PDP_INDEX(va_addr);
    uint64_t pd_index   = VA_GET_PD_INDEX(va_addr);
    uint64_t pt_index   = VA_GET_PT_INDEX(va_addr);
    
    if (!(pml4_ptr[pml4_index] & PRESENT_FLAG)) {
        uint64_t *next_page = get_next_available_mapping_page();
        for (uint64_t i = 0; i < 0x200; i++) {
            next_page[i] = 0;
        }
        pml4_ptr[pml4_index] = PRESENT_FLAG | WRITABLE_FLAG | USER_ACCESS_FLAG;
        pml4_ptr[pml4_index] |= (uint64_t)next_page;
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
        kernel_panic();
    }
}

uint64_t page_get() {

}

uint64_t page_map_multiple(size_t count) {

}