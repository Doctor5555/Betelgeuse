#include <kernel/memory.h>
#include <stdint.h>

/*
 * Bitmap to represent which pages are free
 * in the first 256M of memory.
 * 0 -> Free
 * 1 -> Allocated
 * @TODO: Dynamically allocate a page for this?
 * Or dynamically allocate for all pages?
 */
uint64_t page_bitmap_256M[4096] = { 1 };
/* Total number of pages */
uint64_t page_count;
/* @TODO: Use some system here to represent the rest of memory */

/*
typedef enum {
  EfiReservedMemoryType, No
  EfiLoaderCode, Yes
  EfiLoaderData, Yes
  EfiBootServicesCode, Yes
  EfiBootServicesData, Yes
  EfiRuntimeServicesCode, No
  EfiRuntimeServicesData, No
  EfiConventionalMemory, Yes
  EfiUnusableMemory, No
  EfiACPIReclaimMemory, No
  EfiACPIMemoryNVS, No
  EfiMemoryMappedIO, No
  EfiMemoryMappedIOPortSpace, No
  EfiPalCode, No
  EfiPersistentMemory, No
  EfiMaxMemoryType No
} efi_memory_type;
*/

/*
uint8_t page_get_status(uint64_t ptr) {
    uint64_t idx = ptr >> (6 + 12);
    uint8_t mask = 1 << (ptr & 0b111111000000000000);
    return page_bitmap_256M[idx] & mask;
}
*/
typedef efi_memory_descriptor descriptor;

#define PAGE_STATUS(ptr) ((page_bitmap_256M[(ptr) >> 6] >> ((ptr) & 0b111111)) & 1)
#define DESCRIPTOR(map, index) ((map) + (index) * dsize)

/* @TODO: Rename this function to something better */
uint8_t memory_init(efi_memory_descriptor *mmap, uint64_t dsize, uint64_t dcount) {
    for (uint64_t index = 0; index < dcount; index++) {
        descriptor *d = DESCRIPTOR(mmap, index);

        /* Check whether this descriptor is free or not */
        switch (d->type)
        {
        case EfiReservedMemoryType:
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiUnusableMemory:
        case EfiACPIReclaimMemory:
        case EfiACPIMemoryNVS:
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
        case EfiPalCode:
        case EfiPersistentMemory:
        case EfiMaxMemoryType:
        case 0x10:
            /* code */
            break;

        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory:
            break;
        default:
            break;
        }
    }
}

pageframe_t page_frame_alloc() {
    return 0;
}

void page_frame_free(pageframe_t frame) {

}

/*
 * Returns: a pointer to a contiguous region of memory of a specified size
 * Takes:
 *  - bytes: number of bytes to allocate
 */
void *kmalloc(uint64_t bytes) {

}

/*
 * Returns: none
 * Takes:
 *  - ptr: pointer to the region of memory to free
 */
void kfree(void *ptr) {

}
