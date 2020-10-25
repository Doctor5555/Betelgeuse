#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

typedef uint64_t pageframe_t;

pageframe_t kalloc_page_frame();

void kfree_page_frame(pageframe_t frame);

#endif /* _MEMORY_H */