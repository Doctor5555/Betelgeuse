#ifndef _UEFI_MEM_H
#define _UEFI_MEM_H

#include "common.h"

#include <stdint.h>

#include <efi/boot-services.h>

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

#define OFFSETS_TO_ADDR(pml4, pdp, pd, pt) ((((uint64_t)(pml4) & 0x100UL ? 0xffffULL : 0x0ULL) << CANONICAL_OFFSET) | \
                                             ((uint64_t)(pml4) << PML4_OFFSET) | \
                                             ((uint64_t)(pdp)  << PDP_OFFSET) | \
                                             ((uint64_t)(pd)   << PD_OFFSET) | \
                                             ((uint64_t)(pt)   << PT_OFFSET))

#define ADDR_MASK 0x000FFFFFFFFFF000
#define UINT64MAX 0xFFFFFFFFFFFFFFFF

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

typedef uint64_t physical_addr;
typedef uint64_t virtual_addr;

enum mapping_type {
    MAPPING_TYPE_NONE = 0,
    MAPPING_TYPE_MAP_ALL,
    MAPPING_TYPE_RECURSIVE,

    MAPPING_TYPE_END
};

efi_status init_virtual_mapping(efi_memory_descriptor *mem_map, uint64_t descriptor_count, uint64_t descriptor_size, enum mapping_type mapping_type, uint64_t *mapping_base);
efi_status map_pages(physical_addr src_addr, virtual_addr dest_addr, uint64_t page_count);
efi_status map_page(physical_addr src_addr, virtual_addr dest_addr);

uint64_t get_next_available_mapping_page();

void get_mapping_pointers_and_count(uint64_t *pml4_pointer, uint64_t *used_mapping_page_count);

#endif /* _UEFI_MEM_H */