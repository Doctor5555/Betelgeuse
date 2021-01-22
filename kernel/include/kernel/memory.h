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

#define OFFSETS_TO_ADDR(pml4, pdp, pd, pt) ((((uint64_t)(pml4) & 0x100ULL ? 0xffffULL : 0x0ULL) << CANONICAL_OFFSET) | \
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
 * Deal with a page fault, perhaps by swapping back in the page
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> mapped?????)
 * Takse: none
 */
int8_t page_handle_page_fault();

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
 *  - physical_address: return the physical address that is being unmapped
 */
int8_t page_unmap(virtual_addr virtual_address, physical_addr *physical_address);

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
 *  - boot_table: the boot table passed by 
 *                the loader
 */
int8_t page_allocator_init(struct boot_table *boot_table);

/*
 * Returns: 0 for success, -1 for failure
 * Takes:
 *  - memory_offset: address at which all physical
 *                   memory is re-mapped
 *  - pml4: physical address of the highest page
 *          mapping level.
 */
int8_t page_mapper_init(uint64_t memory_offset, uint64_t *pml4);

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
 *           function to the first allocated page.
 *           Pages are guaranteed to be consecutive.
 */
int8_t page_physical_alloc_multiple(pageframe_t *frame, uint64_t count);

/*
 * Returns: none
 * Takes:
 *  - frame: page frame to free
 */
void page_physical_free(pageframe_t frame);

//@TODO: Better method for this allocation function?
/*
 * Allocate an arbitrary virtual page. It will
 * not be mapped or physically allocated.
 * Current method might not be the best for allocation
 * Currently allocates the first available page after the lowest allocated range.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 */
int8_t page_virtual_alloc(pageframe_t *frame);

/*
 * Allocate a specific virtual page. It will
 * not be mapped or physically allocated.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> already allocated)
 * Takes:
 *  - frame: address to allocate
 */
int8_t page_virtual_alloc_at(pageframe_t frame);

/*
 * Allocate a virtual page at the next available
 * location after a base location.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - base: base address to start from
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_virtual_alloc_after(pageframe_t base, pageframe_t *frame);

/*
 * Allocate a virtual page with a buffer between
 * the new allocation and the provided base. If
 * the base is in an allocation, the buffer will
 * be present between the end of that allocation
 * and the new allocation. If the base is not in
 * an allocation, then the new allocation will
 * simply have a buffer above the base. If another
 * allocation is inside the buffer, or at any point
 * inside the allocation range, the allocation will
 * fail with error code 1.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> target location already mapped)
 * Takes:
 *  - base: address to start search from
 *  - jump_count: buffer beyond allocation to leave
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_virtual_alloc_beyond(pageframe_t base, uint64_t jump_count, pageframe_t *frame);

/*
 * Allocate a virtual page with a minimum buffer 
 * between the new allocation and the previous one,
 * starting at the passed base. If there is an
 * allocation in the way, then it moves to jump
 * beyond that allocation and repeats until a valid
 * location is found.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - base: address to start search from
 *  - jump_count: buffer beyond allocation to leave
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_virtual_alloc_beyond_ex(pageframe_t base, uint64_t jump_count, pageframe_t *frame);

/*
 * Allocate multiple consecutive virtual 
 * pages. They will not be mapped or 
 * physically allocated.
 * Currently allocates the first available
 * page after the lowest allocated range.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> a page is already allocated)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 *  - alloc_count: number of pages to allocate
 */
int8_t page_virtual_alloc_multiple(pageframe_t *frame, uint64_t alloc_count);

/*
 * Allocate multiple consecutive virtual 
 * pages. They will not be mapped or 
 * physically allocated. Propagate forward
 * until adequate space is located.
 * Currently allocates the first available
 * page range of sufficient size after the
 * lowest allocated range.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 *  - alloc_count: number of pages to allocate
 */
int8_t page_virtual_alloc_multiple_ex(pageframe_t *frame, uint64_t alloc_count);

/*
 * Allocate multiple consecutive virtual
 * pages at a specific address.
 * They will not be mapped or physically
 * allocated.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> already allocated)
 * Takes:
 *  - frame: address to allocate at
 *  - alloc_count: number of pages to allocate
 */
int8_t page_virtual_alloc_multiple_at(pageframe_t frame, uint64_t alloc_count);

/*
 * Allocate multiple consecutive virtual pages at
 * the next available location after a base location.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> already allocated (insufficent room at the first available location))
 * Takes:
 *  - base: base address to start from
 *  - alloc_count: number of pages to allocate
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_virtual_alloc_multiple_after(pageframe_t base, uint64_t alloc_count, pageframe_t *frame);

/*
 * Allocate multiple consecutive virtual pages at
 * the next available location with sufficient space
 * after the base location.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - base: base address to start from
 *  - alloc_count: number of pages to allocate
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_virtual_alloc_multiple_after_ex(pageframe_t base, uint64_t alloc_count, pageframe_t *frame);

/*
 * Allocate multiple consecutive virtual pages
 * with a buffer between the new allocation and 
 * the provided base. If the base is in an allocation,
 * the buffer will be present between the end of that
 * allocation and the new allocation. If the base is
 * not in an allocation, then the new allocation will
 * simply have a buffer above the base. If another
 * allocation is inside the buffer, or at any point
 * inside the allocation range, the allocation will
 * fail with error code 1.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> target location already mapped or insufficient area available)
 * Takes:
 *  - base: address to start search from
 *  - alloc_count: number of pages to allocate
 *  - jump_count: buffer beyond allocation to leave
 *  - frame: client-side address set to first
 *           allocated page
 */
int8_t page_virtual_alloc_multiple_beyond(pageframe_t base, uint64_t alloc_count, uint64_t jump_count, pageframe_t *frame);

/*
 * Allocate a virtual page with a buffer between
 * the new allocation and the base. If there is an
 * allocation in the way, then it moves to jump
 * beyond that allocation and repeats until
 * a valid location is found.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - base: address to start search from
 *  - jump_count: buffer beyond allocation to leave
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_virtual_alloc_multiple_beyond_ex(pageframe_t base, uint64_t alloc_count, uint64_t jump_count, pageframe_t *frame);

/*
 * Free a virtual page in the map. It will
 * not be unmapped or physically deallocated.
 * Returns: error code (-1 -> not allocated, 0 -> success)
 * Takes:
 *  - frame: page frame to free
 */
int8_t page_virtual_free(pageframe_t frame);

/*
 * Free a series of contiguous virtual pages
 * They will not be unmapped or physically
 * deallocated.
 * Returns: error code (-1 -> not allocated (all allocated pages in the range will remain allocated), 0 -> success)
 * Takes:
 *  - frame: page frame to free
 *  - count: number of pages to free
 */
int8_t page_virtual_free_multiple(pageframe_t frame, uint64_t count);

/*
 * Allocate a virtual page and map it to a
 * newly allocated physical page
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 */
int8_t page_alloc(pageframe_t *frame);

/*
 * Allocate a specific virtual page and map it to a
 * newly allocated physical page.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> already allocated)
 * Takes:
 *  - frame: address to allocate
 */
int8_t page_alloc_at(pageframe_t frame);

/*
 * Allocate a virtual page at the next available
 * location after a base location and map it to a
 * newly allocated physical page.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - base: base address to start from
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_alloc_after(pageframe_t base, pageframe_t *frame);

/*
 * Allocate a virtual page with a buffer between
 * the new allocation and the provided base and map
 * it to a newly allocated physical page.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> target location already mapped)
 * Takes:
 *  - base: address to start search from
 *  - jump_count: buffer beyond allocation to leave
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_alloc_beyond(pageframe_t base, uint64_t jump_count, pageframe_t *frame);

/*
 * Allocate a virtual page with a buffer between
 * the new allocation and the base and map it to a
 * physical page. If there is an allocation in the 
 * way, then it moves to jump beyond that
 * allocation and repeats until a valid location 
 * is found.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - base: address to start search from
 *  - jump_count: buffer beyond allocation to leave
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_alloc_beyond_ex(pageframe_t base, uint64_t jump_count, pageframe_t *frame);

// @TODO: current method might not be the best for allocation
/*
 * Allocate multiple consecutive virtual 
 * pages and map to physical pages.
 * Currently allocates the first available page after the lowest allocated range.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> a page is already allocated)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 *  - alloc_count: number of pages to allocate
 */
int8_t page_alloc_multiple(pageframe_t *frame, uint64_t alloc_count);

/*
 * Allocate multiple consecutive virtual 
 * pages and mapped and physically allocate.
 * Propagate forward until adequate space is
 * located.
 * Currently allocates the first available
 * page range of sufficient size after the
 * lowest allocated range.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 *  - alloc_count: number of pages to allocate
 */
int8_t page_alloc_multiple_ex(pageframe_t *frame, uint64_t alloc_count);

/*
 * Allocate multiple consecutive virtual
 * pages at a specific address and map to
 * physical pages.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> already allocated)
 * Takes:
 *  - frame: address to allocate at
 *  - alloc_count: number of pages to allocate
 */
int8_t page_alloc_multiple_at(pageframe_t frame, uint64_t alloc_count);

/*
 * Allocate multiple consecutive virtual pages at
 * the next available location after a base location
 * and map to physical pages.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> already allocated (insufficent room at the first available location))
 * Takes:
 *  - base: base address to start from
 *  - alloc_count: number of pages to allocate
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_alloc_multiple_after(pageframe_t base, uint64_t alloc_count, pageframe_t *frame);

/*
 * Allocate multiple consecutive virtual pages at
 * the next available location with sufficient space
 * after the base location and map to physical pages.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - base: base address to start from
 *  - alloc_count: number of pages to allocate
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_alloc_multiple_after_ex(pageframe_t base, uint64_t alloc_count, pageframe_t *frame);

/*
 * Allocate multiple consecutive virtual pages
 * with a buffer between the new allocation and 
 * the provided base and map to physical pages.
 * Returns: error code (-1 -> failed, 0 -> success, 1 -> target location already mapped or insufficient area available)
 * Takes:
 *  - base: address to start search from
 *  - alloc_count: number of pages to allocate
 *  - jump_count: buffer beyond allocation to leave
 *  - frame: client-side address set to first
 *           allocated page
 */
int8_t page_alloc_multiple_beyond(pageframe_t base, uint64_t alloc_count, uint64_t jump_count, pageframe_t *frame);

/*
 * Allocate a virtual page with a buffer between
 * the new allocation and the base and map to 
 * physical pages. If there is an allocation in
 * the way, then it moves to jump beyond that 
 * allocation and repeats until a valid location
 * is found.
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - base: address to start search from
 *  - jump_count: buffer beyond allocation to leave
 *  - frame: client-side address set to allocated
 *           page
 */
int8_t page_alloc_multiple_beyond_ex(pageframe_t base, uint64_t alloc_count, uint64_t jump_count, pageframe_t *frame);

/*
 * Free a virtual page in the map. It will
 * be unmapped and deallocated in the physical
 * page map.
 * Returns: error code (-1 -> not allocated, 0 -> success)
 * Takes:
 *  - frame: page frame to free
 */
void page_free(pageframe_t frame);

/*
 * Free a series of consecutive virtual pages in
 * the map. They will be unmapped and deallocated
 * in the physical page map.
 * If any page is not allocated, all others will
 * be deallocated anyway, but an error code will
 * be returned.
 * Returns: error code (-1 -> not allocated, 0 -> success)
 * Takes:
 *  - frame: page frame to free
 *  - count: number of pages to free
 */
void page_free_multiple(pageframe_t frame, uint64_t count);

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