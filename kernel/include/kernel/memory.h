#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

#include <efi_memory_descriptor.h>

typedef uint64_t pageframe_t;

/*
 * Returns: 1 for success, 0 for failure
 * Takes:
 *  - mmap: pointer to UEFI memory map
 *  - dsize: descriptor size in bytes
 *  - dcount: descriptor count
 */
uint8_t memory_init(efi_memory_descriptor *mmap, uint64_t dsize, uint64_t dcount);

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