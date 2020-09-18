#ifndef _BOOT_TABLE_H
#define _BOOT_TABLE_H

#include <stddef.h>

struct boot_table {
    void *mem_table_ptr;
    size_t mem_desc_size;
    size_t mem_desc_count;
    void *font_ptr;
    unsigned long long kernel_start_ptr;
    struct {
        size_t width;
        size_t height;
        size_t px_per_scan;
        void *framebuffer_base;
        size_t framebuffer_size;
    } graphics_mode;
};

#endif /* _BOOT_TABLE_H */