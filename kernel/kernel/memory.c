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
    uint64_t base_pointer;
    uint64_t _unused;
};

//struct virtual_address_map {
struct virtual_alloc_entry *virtual_address_map;
uint64_t virtual_address_map_length;
uint64_t virtual_address_map_count;
struct virtual_alloc_entry *virtual_address_map_first;
uint64_t *virtual_address_map_free_bitmap;
uint64_t virtual_address_map_free_bitmap_length;
//}

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

    // This is necessary only if the kernel page count is somehow wrong.
    uint64_t i = 0;
    while (result == 1) {
        result = page_map(boot_table->kernel_start_pointer + ((boot_table->kernel_page_count + i) << 12), virtual_address_map);
        i++;
    }
    if (result == -1) {
        page_physical_free(virtual_address_map);
        return -1;
    }
    virtual_address_map = boot_table->kernel_start_pointer + ((boot_table->kernel_page_count + i - 1) << 12);

    result = page_physical_alloc(&virtual_address_map_free_bitmap);
    if (result == -1) {
        uint64_t virtual_address_map_physical_addr;
        page_unmap(virtual_address_map, &virtual_address_map_physical_addr);
        page_physical_free(virtual_address_map_physical_addr);
        return -1;
    }
    result = page_map(boot_table->kernel_start_pointer + (boot_table->kernel_page_count << 12), virtual_address_map_free_bitmap);
    while (result == 1) {
        result = page_map(boot_table->kernel_start_pointer + ((boot_table->kernel_page_count + i) << 12), virtual_address_map_free_bitmap);
        i++;
    }
    if (result == -1) {
        uint64_t virtual_address_map_physical_addr;
        page_unmap(virtual_address_map, &virtual_address_map_physical_addr);
        page_physical_free(virtual_address_map_physical_addr);
        page_physical_free(virtual_address_map_free_bitmap);
        return -1;
    }

    virtual_address_map[0].base_pointer = (uint64_t)boot_table->mapping_base;
    virtual_address_map[0].length = (uint64_t)virtual_address_map + (1 << 12) - (uint64_t)boot_table->mapping_base;
    virtual_address_map[0].next = NULL;
    virtual_address_map_free_bitmap[0] |= 1 << 63;

    virtual_address_map_count = 1;
    virtual_address_map_length = 128;
    virtual_address_map_first = &virtual_address_map[0];
    virtual_address_map_free_bitmap_length = 2;
    
    return 0;
}

/*
 * We have a test set up that overrides these functions to allow
 * the virtual memory management functions to bu run in a user-space
 * environment and check that the algorithms actually work without
 * worrying about kernel-space debugging.
 */
#ifndef TEST_VIRTUAL_MEM

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

#endif /* TEST_VIRTUAL_MEM */


/*for (uint16_t bitmap_index = 0; bitmap_index < virtual_address_map_free_bitmap_length; bitmap_index++) {
    if (virtual_address_map_free_bitmap[bitmap_index] < INT64_MAX) {
        for (int8_t offset = 63; offset >= 0; offset--) {
            if (!(virtual_address_map_free_bitmap[bitmap_index] & (1 << offset))) {
                uint64_t new_index = (bitmap_index << 6) | offset;
                virtual_address_map_free_bitmap[bitmap_index] |= 1 << offset;

                virtual_address_map[new_index].base_pointer = frame;
                virtual_address_map[new_index].length = 1 << 12;
                virtual_address_map[new_index].next = (*tripref)->next;
                (*tripref)->next = &virtual_address_map[new_index];
                return 0;
            }
        }
    }
}*/

/*
 * Solving inline functions with macros!
 * Finds a valid index to insert at and creates a new virtual map entry.
 */
#define INSERT_VIRTUAL_MAP_ENTRY_AND_RETURN(base_pointer_val,  next_pointer_val, length_val) do { \
    for (uint16_t bitmap_index = 0; bitmap_index < virtual_address_map_free_bitmap_length; bitmap_index++) { \
        if (virtual_address_map_free_bitmap[bitmap_index] < INT64_MAX) { \
            for (int8_t offset = 63; offset >= 0; offset--) { \
                if (!(virtual_address_map_free_bitmap[bitmap_index] & (1 << offset))) { \
                    uint64_t new_index = (bitmap_index << 6) | offset; \
                    virtual_address_map_free_bitmap[bitmap_index] |= 1 << offset; \
                    virtual_address_map[new_index].base_pointer = base_pointer_val; \
                    virtual_address_map[new_index].length = length_val; \
                    virtual_address_map[new_index].next = next_pointer_val; \
                    next_pointer_val = &virtual_address_map[new_index]; \
                    return 0; \
                } \
            } \
        } \
    } \
} while (0)

/*
 * Loop through the linked list until we find the correct entry.
 */
#define FIND_ENTRY_WITH_TRIPREF(tripref, frame_val) do { \
    while ((*tripref)->next && (*tripref)->next->base_pointer < frame_val) { \
        tripref = &(*tripref)->next; \
    } \
} while (0)

/*
 * Join the current entry with the next entry if required
 *//*
#define CONDITIONALLY_JOIN_ENTRIES(current_entry) do { \
    if (current_entry->next != 0 && current_entry->base_pointer + current_entry->length \
            == current_entry->next->base_pointer) { \
        uint64_t next_bitmap_location = ((uint64_t)current_entry->next - (uint64_t)virtual_address_map) >> 3; \
        current_entry->length += current_entry->next->length; \
        current_entry->next = current_entry->next->next; \
        virtual_address_map_free_bitmap[next_bitmap_location >> 6] &= ~(1 << (63 - (next_bitmap_location & 0x3f))); \
    } \
} while (0);*/

int8_t page_virtual_alloc(pageframe_t *frame) {
    // @TODO: Figure out how to find a good spot for a new allocation in
    //        a user process that won't be the highest block in memory.
    *frame = virtual_address_map_first->base_pointer + virtual_address_map_first->length;
    if (*frame == 0) {
        return -1; // Probably impossible, but a good idea to check we didn't overflow anyway.
    }
    virtual_address_map_first->length++;
    if (virtual_address_map_first->next != 0 && virtual_address_map_first->base_pointer + virtual_address_map_first->length
            == virtual_address_map_first->next->base_pointer) {
        // Join the two entries together - they now are touching.
        uint64_t next_bitmap_location = ((uint64_t)virtual_address_map_first->next - (uint64_t)virtual_address_map) >> 3;
        virtual_address_map_first->length += virtual_address_map_first->next->length;
        virtual_address_map_first->next = virtual_address_map_first->next->next;
        virtual_address_map_free_bitmap[next_bitmap_location >> 6] &= ~(1 << (63 - (next_bitmap_location & 0x3f)));
    }
    return 0;
}

int8_t page_virtual_alloc_at(pageframe_t frame) {
    struct virtual_alloc_entry **tripref = &virtual_address_map_first;
    FIND_ENTRY_WITH_TRIPREF(tripref, frame);
    if ((*tripref)->base_pointer + (*tripref)->length > frame) {
        return 1; // Location already allocated
    } if ((*tripref)->base_pointer + (*tripref)->length == frame) {
        (*tripref)->length += 1 << 12;
    } else if ((*tripref)->next && (*tripref)->next->base_pointer == frame + (1 << 12)) {
        (*tripref)->next->base_pointer -= (1 << 12);
        (*tripref)->next->length += (1 << 12);
    } else {
        INSERT_VIRTUAL_MAP_ENTRY_AND_RETURN(frame, (*tripref)->next, 1 << 12);
        return -1;
    }
    if ((*tripref)->next && (*tripref)->base_pointer + (*tripref)->length == (*tripref)->next->base_pointer) {
        // Join the two entries together - they now are touching.
        uint64_t next_bitmap_location = ((uint64_t)(*tripref)->next - (uint64_t)virtual_address_map) >> 3;
        (*tripref)->length += (*tripref)->next->length;
        (*tripref)->next = (*tripref)->next->next;
        virtual_address_map_free_bitmap[next_bitmap_location >> 6] &= ~(1 << (63 - (next_bitmap_location & 0x3f)));
    }
    return 0;
}

int8_t page_virtual_alloc_after(pageframe_t base, pageframe_t *frame) {
    struct virtual_alloc_entry **tripref = &virtual_address_map_first;
    FIND_ENTRY_WITH_TRIPREF(tripref, base);
    if ((*tripref)->base_pointer + (*tripref)->length >= base) {
        *frame = (*tripref)->base_pointer + (*tripref)->length;
        (*tripref)->length += 1 << 12;
        return 0;
    } else {
        *frame = base;
        INSERT_VIRTUAL_MAP_ENTRY_AND_RETURN(base, (*tripref)->next, 1 << 12);
    }
    return -1;
}

int8_t page_virtual_alloc_beyond(pageframe_t base, uint64_t jump_count, pageframe_t *frame) {
    if (jump_count == 0) {
        return page_virtual_alloc_after(base, frame);
    }
    struct virtual_alloc_entry **tripref = &virtual_address_map_first;
    FIND_ENTRY_WITH_TRIPREF(tripref, base);

    *frame = (*tripref)->base_pointer + (*tripref)->length + (jump_count << 12);
    if ((*tripref)->next->base_pointer <= *frame &&
        (*tripref)->next->base_pointer + (*tripref)->next->length > *frame) {
        return 1; // Location already allocated
    } else {
        INSERT_VIRTUAL_MAP_ENTRY_AND_RETURN(*frame, (*tripref)->next, 1 << 12);
    }
    return -1;
}

int8_t page_virtual_alloc_beyond_ex(pageframe_t base, uint64_t jump_count, pageframe_t *frame) {
    if (jump_count == 0) {
        return page_virtual_alloc_after(base, frame);
    }
    struct virtual_alloc_entry **tripref = &virtual_address_map_first;
    FIND_ENTRY_WITH_TRIPREF(tripref, base);

    *frame = (*tripref)->base_pointer + (*tripref)->length + (jump_count << 12);
    while ((*tripref)->next->base_pointer <= *frame &&
        (*tripref)->next->base_pointer + (*tripref)->next->length > *frame) {
        tripref = &(*tripref)->next;
        *frame = (*tripref)->base_pointer + (*tripref)->length + (jump_count << 12);
    }
    INSERT_VIRTUAL_MAP_ENTRY_AND_RETURN(*frame, (*tripref)->next, 1 << 12);
    return -1;
}

int8_t page_virtual_alloc_multiple(pageframe_t *frame, uint64_t alloc_count) {
    if (virtual_address_map_first->base_pointer + virtual_address_map_first->length + (alloc_count << 12) > virtual_address_map_first->next->base_pointer) {
        return 1;
    }
    
}

int8_t page_virtual_alloc_multiple_ex(pageframe_t *frame, uint64_t alloc_count) {
    
}

int8_t page_virtual_alloc_multiple_at(pageframe_t frame, uint64_t alloc_count) {

}

int8_t page_virtual_alloc_multiple_after(pageframe_t base, uint64_t alloc_count, pageframe_t *frame) {

}

int8_t page_virtual_alloc_multiple_after_ex(pageframe_t base, uint64_t alloc_count, pageframe_t *frame) {

}

int8_t page_virtual_alloc_multiple_beyond(pageframe_t base, uint64_t alloc_count, uint64_t jump_count, pageframe_t *frame) {

}

int8_t page_virtual_alloc_multiple_beyond_ex(pageframe_t base, uint64_t alloc_count, uint64_t jump_count, pageframe_t *frame) {

}

int8_t page_virtual_free(pageframe_t frame) {
    struct virtual_alloc_entry **tripref = &virtual_address_map_first;
    while ((*tripref)->next->base_pointer < frame) {
        tripref = &(*tripref)->next;
    }
    if (frame >= (*tripref)->base_pointer + (*tripref)->length) {
        // This page is not allocated in the map!
        return -1;
    }
    if ((*tripref)->length == 1) {
        uint64_t free_bitmap_location = ((uint64_t)(*tripref) - (uint64_t)virtual_address_map) >> 3;
        *tripref = (*tripref)->next;
        virtual_address_map_free_bitmap[free_bitmap_location >> 6] &= ~(1 << (63 - (free_bitmap_location & 0x3f)));
    }
    else {
        uint64_t old_length = (*tripref)->length;
        INSERT_VIRTUAL_MAP_ENTRY_AND_RETURN(frame + (1 << 12), (*tripref)->next, old_length - (*tripref)->length - (1 << 12));
    }/*for (uint16_t bitmap_index = 0; bitmap_index < virtual_address_map_free_bitmap_length; bitmap_index++) {
        if (virtual_address_map_free_bitmap[bitmap_index] < INT64_MAX) {
            for (int8_t offset = 63; offset >= 0; offset--) {
                if (!(virtual_address_map_free_bitmap[bitmap_index] & (1 << offset))) {
                    uint64_t new_index = (bitmap_index << 6) | offset;
                    virtual_address_map_free_bitmap[bitmap_index] |= 1 << offset;
                    
                    uint64_t old_length = (*tripref)->length;
                    (*tripref)->length = frame - (*tripref)->base_pointer;
                    virtual_address_map[new_index].base_pointer = frame + (1 << 12);
                    virtual_address_map[new_index].length = old_length - (*tripref)->length - (1 << 12);
                    virtual_address_map[new_index].next = (*tripref)->next;
                    (*tripref)->next = &virtual_address_map[new_index];
                    return 0;
                }
            }
        }
    }*/
    
    //@TODO: Extend number of available entries
    return -1;
}

/*
 * Allocate a virtual page and map it to a physical
 * page
 * Returns: error code (-1 -> failed, 0 -> success)
 * Takes:
 *  - frame: pointer to page_frame variable
 *           on the client side. Set in the
 *           function to the allocated page
 */
int8_t page_alloc(pageframe_t *frame);

/*
 * Free a virtual page in the map. It will
 * be unmapped and deallocated in the physical
 * page map.
 * Returns: none
 * Takes:
 *  - frame: page frame to free
 */
void page_free(pageframe_t frame);

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
