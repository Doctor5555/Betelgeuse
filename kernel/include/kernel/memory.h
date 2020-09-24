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
    unsigned long long zero: 1;
    unsigned long long IGN_1: 1;
    unsigned long long AVL: 3;
    unsigned long long addr: 35;
    unsigned long long available: 11;
    unsigned long long NX: 1;
};

struct PD_entry {
    unsigned long long P: 1;
    unsigned long long R_W: 1;
    unsigned long long U_S: 1;
    unsigned long long PWT: 1;
    unsigned long long PCD: 1;
    unsigned long long A: 1;
    unsigned long long IGN_2: 1;
    unsigned long long zero: 1;
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

typedef struct addr {
    unsigned long long page_offset:12;
    unsigned long long pt_offset  : 9;
    unsigned long long pd_offset  : 9;
    unsigned long long pdp_offset : 9;
    unsigned long long pml4_offset: 9;
    unsigned long long sign_extend:16;
} addr;

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