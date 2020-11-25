#include <kernel/memory.h>
#include <stdint.h>

/*
 * Bitmap to represent which pages are free\n
 * in the first 1 GiB of memory.\n
 * 0 -> Free\n
 * 1 -> Allocated
 * @TODO: Dynamically allocate a page for this?\n
 * Or dynamically allocate for all pages?\n
 */
uint64_t page_bitmap_1G[4096] = { 1 };

#define BITMAP_ARRAY_LENGTH 4096
#define BITMAP_MAX_ADDR 0x40000000

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
    return page_bitmap_1G[idx] & mask;
}
*/

//typedef efi_memory_descriptor descriptor;

#define PAGE_STATUS(ptr) ((page_bitmap_1G[(ptr) >> 6] >> ((ptr) & 0b111111)) & 1)
#define DESCRIPTOR(map, index) ((map) + (index) * dsize)

/* @TODO: Rename this function to something better */
int8_t memory_init(efi_memory_descriptor *mmap, uint64_t dsize, uint64_t dcount) {
    for (uint64_t index = 0; index < dcount; index++) {
        efi_memory_descriptor *d = DESCRIPTOR(mmap, index);

        uint8_t finished_1G = 0;

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
        case EfiConventionalMemory: {
            for (uint64_t i = 0; i < d->number_of_pages; i++) {
                uint64_t addr = d->physical_start >> 12 + i;
                uint64_t bitmap_index = addr >> 6;
                uint8_t bitmap_offset = addr & 0b111111;
                if (bitmap_index > BITMAP_ARRAY_LENGTH) {
                    finished_1G = 1;
                    break;
                }
                page_bitmap_1G[bitmap_index] ^= 1 << bitmap_offset;
            }
            break;
        }
        default:
            break;
        }

        if (finished_1G) {
            break;
        }
    }
    return 0;
}

int8_t page_frame_alloc(pageframe_t *frame) {
    for (uint16_t bitmap_index = 0; bitmap_index < BITMAP_ARRAY_LENGTH; bitmap_index++) {
        if (page_bitmap_1G[bitmap_index] < INT64_MAX) {
            for (int8_t offset = 63; offset >= 0; offset--) {
                if (!(page_bitmap_1G[bitmap_index] & (1 << offset))) {
                    *frame = (bitmap_index << 18) | (offset << 12);
                    page_bitmap_1G[bitmap_index] ^= 1 << offset;
                    return 0;
                }
            }
        }
    }
    return -1;
}

void page_frame_free(pageframe_t frame) {
    if ((frame < BITMAP_MAX_ADDR) && !PAGE_STATUS(frame >> 12)) {
        page_bitmap_1G[frame >> 18] ^= 1 << ((frame >> 12) & 0b111111);
    }
}

/*
 * Returns: a pointer to a contiguous region of memory of a specified size
 * Takes:
 *  - bytes: number of bytes to allocate
 */
void *kmalloc(uint64_t bytes) {
    return 0;
}

/*
 * Returns: none
 * Takes:
 *  - ptr: pointer to the region of memory to free
 */
void kfree(void *ptr) {

}
