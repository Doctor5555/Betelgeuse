#include <kernel/memory.h>

static struct PML4_entry *memory_map;


void page_map(uint64_t virtual, uint64_t physical) {
    uint16_t pml4_index = (virtual >> 39) & 0b111111111;
    uint16_t pdp_index = (virtual >> 30) & 0b111111111;
    uint16_t pd_index = (virtual >> 21) & 0b111111111;
    uint16_t pt_index = (virtual >> 12) & 0b111111111;

    struct PDP_entry *pdp = memory_map[pml4_index].addr << 12 - (1 << 12);
    struct PD_entry_2M *pd = pdp[pdp_index].addr << 12 - (1 << 12);
    //struct PT_entry *pt = pd[pd_index].addr << 12 - (1 << 12);
    //struct PT_entry *page_entry = pt + pt_index;
    //page_entry->addr = 
}

uint64_t page_get() {

}

uint64_t page_map_multiple(size_t count) {

}