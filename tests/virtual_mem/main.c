#include <kernel/memory.h>

#include <stdlib.h>
#include <stdio.h>

int8_t page_mapper_init(uint64_t memory_offset, uint64_t *pml4) {
    return 0;
}

int8_t page_physical_alloc(pageframe_t *frame) {
    *frame = (uint64_t)malloc(4096);
    return 0;
}

void page_physical_free(pageframe_t frame) {
    free(frame);
}

int8_t page_map(virtual_addr virtual_address, physical_addr physical_addr) {
    return 0;
}

int8_t page_unmap(virtual_addr virtual_address, physical_addr *physical_address) {
    *physical_address = 0;
    return 0;
}

struct virtual_alloc_entry {
    struct virtual_alloc_entry *next;
    uint64_t length;
    uint64_t base_pointer;
    uint64_t _unused;
};

//struct virtual_address_map {
struct virtual_alloc_entry *virtual_address_map;
uint64_t virtual_address_map_length;
uint64_t virtual_address_map_count;
struct virtual_alloc_entry *virtual_address_map_first;
uint64_t *virtual_address_map_free_bitmap;
uint64_t virtual_address_map_free_bitmap_length;

void main() {
    printf("Test!\n");

    page_physical_alloc(&virtual_address_map);
    page_physical_alloc(&virtual_address_map_free_bitmap);

    virtual_address_map[0].base_pointer = (uint64_t)0x00080000;
    virtual_address_map[0].length = (5 << 12);
    virtual_address_map[0].next = NULL;
    virtual_address_map_free_bitmap[0] |= 1 << 63;

    virtual_address_map_count = 1;
    virtual_address_map_length = 128;
    virtual_address_map_first = &virtual_address_map[0];
    virtual_address_map_free_bitmap_length = 2;

    
}