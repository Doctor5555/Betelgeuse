#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>
#include <stddef.h>

struct PML4_entry {
    unsigned long long P: 1;
    unsigned long long R_W: 1;
    unsigned long long U_S: 1;
    unsigned long long PWT: 1;
    unsigned long long PCD: 1;
    unsigned long long A: 1;
    unsigned long long IGN: 1;
    unsigned long long MBZ: 2;
    unsigned long long AVL: 3;
    unsigned long long addr: 35;
    unsigned long long available: 11;
    unsigned long long NX: 1;
};

struct PDP_entry {
    unsigned long long P: 1;
    unsigned long long R_W: 1;
    unsigned long long U_S: 1;
    unsigned long long PWT: 1;
    unsigned long long PCD: 1;
    unsigned long long A: 1;
    unsigned long long IGN_2: 1;
    unsigned long long PS: 1;
    unsigned long long IGN_1: 1;
    unsigned long long AVL: 3;
    unsigned long long addr: 35;
    unsigned long long available: 11;
    unsigned long long NX: 1;
};

struct PD_entry_2M {
    unsigned long long P: 1;
    unsigned long long R_W: 1;
    unsigned long long U_S: 1;
    unsigned long long PWT: 1;
    unsigned long long PCD: 1;
    unsigned long long A: 1;
    unsigned long long D: 1;
    unsigned long long PS: 1;
    unsigned long long G: 1;
    unsigned long long AVL: 3;
    unsigned long long PAT: 1;
    unsigned long long reserved: 8;
    unsigned long long addr: 31;
    unsigned long long available: 7;
    unsigned long long MPK_opt: 3;
    unsigned long long NX: 1;
};

struct PD_entry_4K {
    unsigned long long P: 1;
    unsigned long long R_W: 1;
    unsigned long long U_S: 1;
    unsigned long long PWT: 1;
    unsigned long long PCD: 1;
    unsigned long long A: 1;
    unsigned long long IGN_2: 1;
    unsigned long long PS: 1;
    unsigned long long IGN_1: 1;
    unsigned long long AVL: 3;
    unsigned long long addr: 35;
    unsigned long long available: 11;
    unsigned long long NX: 1;
};

struct PT_entry {
    unsigned long long P: 1;
    unsigned long long R_W: 1;
    unsigned long long U_S: 1;
    unsigned long long PWT: 1;
    unsigned long long PCD: 1;
    unsigned long long A: 1;
    unsigned long long D: 1;
    unsigned long long PAT: 1;
    unsigned long long G: 1;
    unsigned long long AVL: 3;
    unsigned long long addr: 35;
    unsigned long long available: 7;
    unsigned long long CR4_PKE_dep: 4;
    unsigned long long NX: 1;
};

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

#define OFFSETS_TO_ADDR(pml4, pdp, pd, pt) ((((uint64_t)(pml4) & 0x8000 ? 0xffffUL : 0x0UL) << CANONICAL_OFFSET) | \
                                             ((uint64_t)(pml4) << PML4_OFFSET) | \
                                             ((uint64_t)(pdp)  << PDP_OFFSET) | \
                                             ((uint64_t)(pd)   << PD_OFFSET) | \
                                             ((uint64_t)(pt)   << PT_OFFSET))

/* 
 * Returns: none
 * Takes:
 *  - virtual: virtual address to map to
 *  - physical: physical address to map from
 */
void page_map(uint64_t virtual, uint64_t physical);

/* 
 * Returns: next available physical page
 * Takes: none
 */
uint64_t page_get();

/* 
 * Returns: first virtual address of continuous mapping
 * Takes:
 *  - count: number of continuous pages to map
 */
uint64_t page_map_multiple(size_t count);

#endif /* _MEMORY_H */