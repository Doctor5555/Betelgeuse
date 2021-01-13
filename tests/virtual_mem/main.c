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

void main() {
    printf("Test!\n");
}