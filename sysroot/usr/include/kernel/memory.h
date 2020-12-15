#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>
#include <stddef.h>

#include <efi_memory_descriptor.h>
#include <bootloader/boot_table.h>

#define PAGE_MASK(x)           ((uint64_t)(x) & 0x0000000000000fff)
#define PT_MASK(x)             ((uint64_t)(x) & 0x00000000001ff000)
#define PD_MASK(x)             ((uint64_t)(x) & 0x000000003fe00000)
#define PDP_MASK(x)            ((uint64_t)(x) & 0x0000007fc0000000)
#define PML4_MASK(x)           ((uint64_t)(x) & 0x0000ff8000000000)
#define CANONICAL_SIGN_EXTEND(x) ((uint64_t)(x) & 0xffff000000000000)

#define PT_OFFSET 12
#define PD_OFFSET 21
#define PDP_OFFSET 30
#define PML4_OFFSET 39
#define CANONICAL_OFFSET 48

#define VA_GET_PML4_INDEX(x) (PML4_MASK(x) >> PML4_OFFSET)
#define VA_GET_PDP_INDEX(x)  (PDP_MASK(x) >> PDP_OFFSET)
#define VA_GET_PD_INDEX(x)   (PD_MASK(x) >> PD_OFFSET)
#define VA_GET_PT_INDEX(x)   (PT_MASK(x) >> PT_OFFSET)

#define OFFSETS_TO_ADDR(pml4, pdp, pd, pt) ((((uint64_t)(pml4) & 0x200 ? 0xffffUL : 0x0UL) << CANONICAL_OFFSET) | \
                                             ((uint64_t)(pml4) << PML4_OFFSET) | \
                                             ((uint64_t)(pdp)  << PDP_OFFSET) | \
                                             ((uint64_t)(pd)   << PD_OFFSET) | \
                                             ((uint64_t)(pt)   << PT_OFFSET))

#define ADDR_MASK 0x000FFFFFFFFFF000

#define PRESENT_FLAG       (1 << 0)
#define WRITABLE_FLAG      (1 << 1)
#define USER_ACCESS_FLAG   (1 << 2)
#define WRITE_THROUGH_FLAG (1 << 3)
#define NO_CACHE_FLAG      (1 << 4)
#define ACCESSED_FLAG      (1 << 5)
#define DIRTY_FLAG         (1 << 6)
#define PAGE_SIZE_FLAG     (1 << 7)
#define GLOBAL_FLAG        (1 << 8)
#define NO_EXECUTE_FLAG    (1 << 63)

#define AVAILABLE_HIGH_MASK   0x07f0000000000000
#define AVAILABLE_LOW_MASK    0x0000000000000700
#define AVAILABLE_HIGH_OFFSET 0x34
#define AVAILABLE_LOW_OFFSET  0x9

typedef uint64_t physical_addr;
typedef uint64_t virtual_addr;

/*
 * 64-bit address of a page
 */
typedef uint64_t pageframe_t;

/*
 * Map a page in the active page table
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> already mapped)
 * Takes:
 *  - virtual: virtual address to map to
 *  - physical: physical address to map from
 */
int8_t page_map(virtual_addr virtual_address, physical_addr physical_addr);

/*
 * Map a page in a specific page table DO NOT USE
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> already mapped)
 * Takes:
 *  - virtual: virtual address to map to
 *  - physical: physical address to map from
 * This only works if all the tables are identity mapped in the current map
 */
int8_t page_map_in_table(void *pml4, virtual_addr virtual_address, physical_addr physical_addr);

/*
 * Returns: error code (-1 -> failed, 0 -> success, >= 1 -> index of already mapped page + 1)
 * Takes:
 *  - count: number of continuous pages to map
 *  - virtual: virtual address to map to
 *  - physical: array of physical pages to map
 */
int64_t page_map_multiple(uint64_t count, virtual_addr virtual_address, physical_addr *physical_addr);

/*
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> not mapped)
 * Takes:
 *  - virtual_address: address to remove from the mapping table
 */
int8_t page_unmap(virtual_addr virtual_address);

// @TODO Implement page_map_create properly
/*
 * Creates a new page map
 * NOT IMPLEMENTED YET
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - map: OUT new page map physical address
 */
int8_t page_map_create(physical_addr *map);

/*
 * Set the active page map
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - pml4_pointer: pml4 to load into cr4
 */
int8_t page_map_set(void *pml4_pointer);

/*
 * Returns: 0 for success, -1 for failure
 * Takes:
 *  - mmap: pointer to UEFI memory map
 *  - dsize: descriptor size in bytes
 *  - dcount: descriptor count
 */
int8_t memory_init(struct boot_table *boot_table);

/*
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 */
int8_t page_physical_alloc(pageframe_t *frame);

/*
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated pages.
 *           Pages are guaranteed to be consecutive
 */
int8_t page_physical_alloc_multiple(pageframe_t *frame, uint64_t count);

/*
 * Returns: none
 * Takes:
 *  - frame: page frame to free
 */
void page_physical_free(pageframe_t frame);

/*
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 */
int8_t page_virtual_alloc(pageframe_t *frame);

/*
 * Returns: none
 * Takes:
 *  - frame: page frame to free
 */
void page_virtual_free(pageframe_t frame);

/*
 * Returns: a pointer to a contiguous region of memory of a specified size
 * Takes:
 *  - bytes: number of bytes to allocate
 */
void *kmalloc(uint64_t bytes);

/*
 * Returns: none
 * Takes:
 *  - pointer: pointer to the region of memory to free
 */
void kfree(void *pointer);

#endif /* _MEMORY_H */