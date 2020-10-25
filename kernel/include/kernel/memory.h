#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

typedef uint64_t pageframe_t;

/*
 * Returns: a newly allocated page frame
 * Takes: none
 */
pageframe_t page_frame_alloc();

/*
 * Returns: none
 * Takes:
 *  - frame: page frame to free
 */
void page_frame_free(pageframe_t frame);

/*
 * Returns: a pointer to a contiguous region of memory of a specified size
 * Takes:
 *  - bytes: number of bytes to allocate
 */
void *kmalloc(uint64_t bytes);

/*
 * Returns: none
 * Takes:
 *  - ptr: pointer to the region of memory to free
 */
void kfree(void *ptr);

#endif /* _MEMORY_H */