#ifndef _BOOT_TABLE_H
#define _BOOT_TABLE_H

#include <stdint.h>

struct boot_table {
    void *mem_table_ptr;
    uint64_t mem_desc_size;
    uint64_t mem_desc_count;
    void *font_ptr;
    unsigned long long kernel_start_ptr;
    uint64_t next_available_mapping_page;
    uint64_t available_mapping_page_count;
    uint64_t *pml4_ptr;
    struct {
        uint64_t width;
        uint64_t height;
        uint64_t px_per_scan;
        void *framebuffer_base;
        uint64_t framebuffer_size;
    } graphics_mode;
};

#endif /* _BOOT_TABLE_H */