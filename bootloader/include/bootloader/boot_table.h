#ifndef _BOOT_TABLE_H
#define _BOOT_TABLE_H

#include <stdint.h>

struct boot_table {
    void *mem_table_pointer;
    uint64_t mem_desc_size;
    uint64_t mem_desc_count;
    void *mapping_base;
    void *font_pointer;
    uint64_t kernel_start_pointer;
    uint64_t kernel_page_count;
    uint64_t *pml4_pointer;
    uint64_t used_mapping_page_count;
    struct {
        uint64_t width;
        uint64_t height;
        uint64_t px_per_scan;
        void *framebuffer_base;
        uint64_t framebuffer_size;
    } graphics_mode;
};


#endif /* _BOOT_TABLE_H */