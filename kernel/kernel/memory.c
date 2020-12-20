#include <kernel/memory.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Bitmap to represent which pages are free\n
 * 0 -> Free\n
 * 1 -> Allocated
 */
uint64_t *page_bitmap;
uint64_t page_bitmap_length;

/*
 * Structure represents a block of virtual pages
 * Members:
 *  - next: next element in the linked list
 *  - base_pointer: base address of the virtual region
 *  - length: number of pages in the region
 *  - is_allocated: 0 -> free, 1 -> allocated
 *  - _unused_1, _unused_2, _unused_3: 56 bit pack to nearest 64-bit size
 * Size: 32 bytes
 */
struct virtual_alloc_entry {
    struct virtual_alloc_entry *next;
    uint64_t length;
    uint64_t is_allocated: 1;
    uint64_t _unused: 7;
    uint64_t base_pointer: 56;
};

struct virtual_alloc_entry *virtual_address_map;
uint64_t virtual_address_map_length;
uint64_t *virtual_address_map_free_bitmap;

/*
 * Structure represents the address space of a process
 */
struct address_space {
    uint64_t page_map_pointer;
    struct virtual_alloc_entry *virtual_map_pointer;
    uint64_t virtual_address_map_length;
    uint64_t *virtual_address_map_free_bitmap;
};

/*
uint8_t page_get_status(uint64_t pointer) {
    uint64_t idx = pointer >> (6 + 12);
    uint8_t mask = 1 << (pointer & 0b111111000000000000);
    return page_bitmap[idx] & mask;
}
*/
#define PAGE_STATUS(pointer) ((page_bitmap[(pointer) >> 6] >> ((pointer) & 0b111111)) & 1)
#define DESCRIPTOR(map, index) ((efi_memory_descriptor*)((uint64_t)(map) + (index) * dsize))

/* @TODO: Rename this function to something better */
int8_t memory_init(struct boot_table *boot_table) {
    efi_memory_descriptor *mmap = boot_table->mem_table_pointer;
    uint64_t dsize = boot_table->mem_desc_size;
    uint64_t dcount = boot_table->mem_desc_count;
    uint64_t required_pages = 32; // @TODO: Calculate this in case I forget
    printf("DESCRIPTOR(mmap, dcount - 1)->physical_start: %#018llx\n\r", DESCRIPTOR(mmap, dcount - 1)->physical_start);
    printf("DESCRIPTOR(mmap, dcount - 1)->number_of_pages: %#018llx\n\r", DESCRIPTOR(mmap, dcount - 1)->number_of_pages);
    uint64_t reserved_pages = UINT64_MAX;
    uint64_t reserved_index = 0;

    struct {
        uint8_t font_buffer : 1;
        uint8_t kernel : 1;
    } type_correction_to_do = { 0, 0 };
    for (uint64_t i = 0; i < dcount; i++) {
        efi_memory_descriptor *d = DESCRIPTOR(mmap, i);
        if (d->physical_start <= boot_table->font_pointer && type_correction_to_do.font_buffer) {
            d->type = 0x12;
            type_correction_to_do.font_buffer = 0;
        }
        if (d->physical_start <= boot_table->kernel_start_pointer && type_correction_to_do.kernel) {
            d->type = 0x12;
            type_correction_to_do.kernel = 0;
        }
        switch (d->type) {
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory:
            if (d->number_of_pages >= required_pages && d->number_of_pages < reserved_pages) {
                page_bitmap = d->physical_start;
                reserved_pages = d->number_of_pages;
                reserved_index = i;
                printf("Base: %#018llx, Required: %lld, Reserved: %lld, Index: %lld\n\r", page_bitmap, required_pages, reserved_pages, i);
            }
        default:
            break;
        }
    }
    if (reserved_pages == UINT64_MAX) {
        // No valid location for the physical memory bitmap found.
        return -1;
    }
    DESCRIPTOR(mmap, reserved_index)->type = 0x11;
    printf("Allocated location: %#018llx\n\r", page_bitmap);
    // Multiply by 512 - number of uint64_t s in a page - to get full array length.
    page_bitmap_length = required_pages << 9;
    for (uint64_t i = 0; i < page_bitmap_length; i++) {
        page_bitmap[i] = 0xFFFFFFFFFFFFFFFF;
    }
    for (uint64_t index = 0; index < dcount; index++) {
        efi_memory_descriptor *d = DESCRIPTOR(mmap, index);
        /* Check whether this descriptor is free or not */
        switch (d->type)
        {
        case 0x10: { /* custom setting for kernel page maps */
            /*for (uint64_t i = 0; i < boot_table->used_mapping_page_count; i++) {
                uint64_t addr = d->physical_start >> 12 + i;
                uint64_t bitmap_index = addr >> 6;
                uint8_t bitmap_offset = addr & 0b111111;
                page_bitmap[bitmap_index] |= 1 << bitmap_offset;
            }*/
            for (uint64_t i = boot_table->used_mapping_page_count; i < d->number_of_pages; i++) {
                uint64_t addr = d->physical_start >> 12 + i;
                uint64_t bitmap_index = addr >> 6;
                uint8_t bitmap_offset = addr & 0b111111;
                page_bitmap[bitmap_index] &= ~(1 << bitmap_offset);
            }
            break;
        }
        case 0x11: { /* custom setting for page bitmap */
            /*for (uint64_t i = 0; i < required_pages; i++) {
                uint64_t addr = d->physical_start >> 12 + i;
                uint64_t bitmap_index = addr >> 6;
                uint8_t bitmap_offset = addr & 0b111111;
                page_bitmap[bitmap_index] |= 1 << bitmap_offset;
            }*/
            for (uint64_t i = required_pages; i < d->number_of_pages; i++) {
                uint64_t addr = d->physical_start >> 12 + i;
                uint64_t bitmap_index = addr >> 6;
                uint8_t bitmap_offset = addr & 0b111111;
                page_bitmap[bitmap_index] &= ~(1 << bitmap_offset);
            }
            break;
        }

        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory: {
            // @TODO: Optimise this to set whole entries if it spans a large enough range
            for (uint64_t i = 0; i < d->number_of_pages; i++) {
                uint64_t addr = d->physical_start >> 12 + i;
                uint64_t bitmap_index = addr >> 6;
                uint8_t bitmap_offset = addr & 0b111111;
                page_bitmap[bitmap_index] &= ~(1 << bitmap_offset);
            }
            break;
        }

        /*case EfiReservedMemoryType:
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
        case 0x12:*/
        default: {
            /*for (uint64_t i = 0; i < d->number_of_pages; i++) {
                uint64_t addr = d->physical_start >> 12 + i;
                uint64_t bitmap_index = addr >> 6;
                uint8_t bitmap_offset = addr & 0b111111;
                page_bitmap[bitmap_index] |= 1 << bitmap_offset;
            }*/
            break;
        }
        }
    }
    page_mapper_init(boot_table->mapping_base, boot_table->pml4_pointer);
    
    int8_t result = page_physical_alloc(&virtual_address_map);
    if (result == -1) {
        return -1;
    }
    result = page_map(boot_table->kernel_start_pointer + (boot_table->kernel_page_count << 12), virtual_address_map);
    uint64_t i = 0;
    while (result == 1) {
        result = page_map(boot_table->kernel_start_pointer + ((boot_table->kernel_page_count + i) << 12), virtual_address_map);
        i++;
    }
    if (result == -1) {
        return -1;
    }
    virtual_address_map = boot_table->kernel_start_pointer + ((boot_table->kernel_page_count + i) << 12);
    printf("virtual_address_map:%#018llx\n\r", virtual_address_map);
    return 0;
}

int8_t page_physical_alloc(pageframe_t *frame) {
    for (uint16_t bitmap_index = 0; bitmap_index < page_bitmap_length; bitmap_index++) {
        if (page_bitmap[bitmap_index] < INT64_MAX) {
            for (int8_t offset = 63; offset >= 0; offset--) {
                if (!(page_bitmap[bitmap_index] & (1 << offset))) {
                    *frame = (bitmap_index << 18) | (offset << 12);
                    page_bitmap[bitmap_index] ^= 1 << offset;
                    return 0;
                }
            }
        }
    }
    return -1;
}

int8_t page_physical_alloc_multiple(pageframe_t *frame, uint64_t count) {
    uint8_t c = 0;
    for (uint16_t bitmap_index = 0; bitmap_index < page_bitmap_length; bitmap_index++) {
        if (page_bitmap[bitmap_index] < INT64_MAX) {
            for (int8_t offset = 63; offset >= 0; offset--) {
                if (!(page_bitmap[bitmap_index] & (1 << offset))) {
                    c++;
                } else {
                    c = 0;
                }
                if (c == count) {
                    *frame = ((bitmap_index << 18) | (offset << 12)) - (count << 12);
                    page_bitmap[bitmap_index] ^= 1 << offset;
                    return 0;
                }
            }
        } else {
            c = 0;
        }
    }
    return -1;
}

void page_physical_free(pageframe_t frame) {
    // @TODO: Add extra verification that this page actually exists
    if ((frame >> 18) < page_bitmap_length && !PAGE_STATUS(frame >> 12)) {
        page_bitmap[frame >> 18] ^= 1 << ((frame >> 12) & 0b111111);
    }
}

int8_t page_virtual_alloc(pageframe_t *frame) {
    return -1;
}

void page_virtual_free(pageframe_t frame) {
    return 0;
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
