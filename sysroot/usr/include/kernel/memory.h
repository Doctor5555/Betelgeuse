#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

#include <efi_memory_descriptor.h>

/*
 * 64-bit address of a page
 */
typedef uint64_t pageframe_t;

/*
 * Returns: 0 for success, -1 for failure
 * Takes:
 *  - mmap: pointer to UEFI memory map
 *  - dsize: descriptor size in bytes
 *  - dcount: descriptor count
 */
int8_t memory_init(efi_memory_descriptor *mmap, uint64_t dsize, uint64_t dcount);

/*
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 */
int8_t page_frame_alloc(pageframe_t *frame);

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