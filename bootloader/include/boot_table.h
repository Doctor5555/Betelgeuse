#ifndef _BOOT_TABLE_H
#define _BOOT_TABLE_H

typedef unsigned long long size_t;

struct boot_table {
    void *mem_table_ptr;
    void *font_ptr;
    struct {
        size_t width;
        size_t height;
        size_t px_per_scan;
        void *framebuffer_base;
        size_t framebuffer_size;
    } graphics_mode;
};

#endif /* _BOOT_TABLE_H */