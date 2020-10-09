#include <kernel/boot_table.h>
#include <kernel/tty.h>
#include <kernel/memory.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include <efi_memory_descriptor.h>

struct boot_table boot_table;

void __attribute__((__section__(".start"))) early_kmain(struct boot_table *boot_table_ptr) {
    boot_table = *boot_table_ptr;
    terminal_init(boot_table_ptr);
}

__attribute__ ((constructor)) void constructor_test() {
    terminal_writestring((unsigned char *)"Hello, Constructor World!\n\r");
}

__attribute__ ((noinline))
int ssp_test(const char *test) {
    int len = strlen(test);
    char dest[3];
    for (int i = 0; i < len; i++) {
        dest[i] = test[i];
    }
    return dest[2] & test[1];
}

u64 kmain() {
    if (boot_table.kernel_start_ptr == 0) {
        printf("Kernel start ptr is zero! This should not be the case!\n\r");
    } else {
        printf("boot_table->kernel_start_ptr: %#018llx\n\r", boot_table.kernel_start_ptr);
    }

    u32 fg = 0xFFFFFF;
    u32 bg = 0;
    terminal_setcolour(fg, bg);
    u32 x = 0;
    u32 y = 7;
    terminal_setcolour(0xDCDCDC, bg);
    for (unsigned char i = 0x00; i < 0xFF; i++) {
        terminal_putchar(i);
        x++;
        if (x > 0xF) {
            x = 0;
            y++;
        }
        //terminal_write(&i, 1);
    }
    printf("\n\rHello, Printf World %#05x!\n\r", 5000);
    printf("\n\rHello, Printf World 2!\n\r");

    ssp_test("12");
    printf("Done ssp test 1 with 12!\n\r");
    /*
    // These two lines of code intentionally trigger the stack smash protector to test that it works.
    ssp_test("Hello, Worlddsjkfghaskdhjfgasuiytg43iu57yqt34787qahgaeit784hit87f4yo87qg43gahwr87w4gi87orwaotw47tco27t4x8ba74c2684tcyr87antn24x3ntx87a3ywcknraw7tykvw37iayrxieugfau7wxt478aigweszt78hbawesgviydiawbszyiwtbhkgesivdo7GEUabk2qjuagiwefubkh2qiagwye78FIYwahtbj2qagiwyeFWahbjt2qwegiybht4kegiwySwabhkt42gisVYDhbrk23kfewgivydlstgeybhk2qktegwyiSDV&Tigeuwkt4bh2wgurls7dvtˆ©ukwbht42giuwe79tvdigeukbht2.37dtv9igeukbht4eguiktbhj4eguwisd;'opu[gpra[2]3otu2 ]wi4yvnOY@$^({*BYNVT&@$*NCN&T@{$98cY&#{N(*O&V[t48vwyt8v'w4tN*NYc984wty!");
    printf("Done ssp test 2 with Hello, World!!\n\r");
    */

/*
    u64 b = 0;
    __asm__ __volatile__ (
        "movq %%cr4, %0;" : "=r" (b) :
    );
    printf("Contents of cr4: %#llx\n\r", b);
    printf("PCIDE: %#llx\n\r", b & (0x1 << 17));
*/

    //efi_memory_descriptor *memory_map = boot_table.mem_table_ptr;
    //u64 descriptor_size = boot_table.mem_desc_size;
    //u64 descriptor_count = boot_table.mem_desc_count;

    //terminal_cursor(0, 0);

    u64 a = 0;
    __asm__ __volatile__ (
        "movq %%cr3, %0;" : "=r" (a) :
    );
    printf("Contents of cr3: %#llx\n\r", a);
    
    /*
    printf("Memory descriptor count: %#018llx\n\r", descriptor_count);
    u64 page_count = 0;
    for (size_t i = 0; i < descriptor_count; i++) {
        efi_memory_descriptor *descriptor = boot_table.mem_table_ptr + i * descriptor_size;
        page_count += descriptor->NumberOfPages;
        if (descriptor->Attribute & 0x8000000000000000)
            printf("index: %2d, attribute: 0x8%015llx, num pages: %#010llx, physical start: %#010llx, virt start: %#010llx, type: %#010x\n\r", 
                    i, descriptor->Attribute & 0x7fffffffffffffff, descriptor->NumberOfPages, descriptor->PhysicalStart, descriptor->VirtualStart, descriptor->Type);
        else
            printf("index: %2d, attribute: %#018llx, num pages: %#010llx, physical start: %#010llx, virt start: %#010llx, type: %#010x\n\r", 
                    i, descriptor->Attribute, descriptor->NumberOfPages, descriptor->PhysicalStart, descriptor->VirtualStart, descriptor->Type);
    }

    printf("Total number of pages: %d, %#llx\n\r", page_count, page_count);
    //*/

    struct PML4_entry *pml4 = (struct PML4_entry*)a;

    uint8_t *new_map = (uint8_t *)0x1400000UL;
    uint8_t *old_map = (uint8_t *)0x7bff000UL;
    uint64_t map_size = 0x201 << 12;

    for (uint64_t i = 0; i < map_size; i++) {
        new_map[i] = old_map[i];
    }

    pml4 = (void*)new_map + ((void*)pml4 - (void*)old_map);
    printf("pml4: %#018llx\n\r", pml4);

    printf("CR4 address: %#018llx, and -(1 << 12): %#018llx\n\r", pml4, (u64)pml4 - (1 << 12));
    //pml4 = (u64)pml4 - (1 << 12);
    printf("CR4 address: %#018llx, and -(1 << 12): %#018llx\n\r", pml4, (u64)pml4 - (1 << 12));

#define old_addr_to_new_map(x) ((uint64_t)((void*)new_map + ((void*)((x) << 12) - (void*)old_map)) >> 12)

    //*
    u64 present_PML4_entries = 0;
    u64 present_PDP_entries = 0;
    u64 present_PD_entries = 0;
    u64 present_PD_entries_2 = 0;
    u64 present_PT_entries = 0;
    char found_non_present_pt = 0;
    char found_last_pt = 0;
    char found_last_pd = 0;
    uint64_t highest_addr = 0;
    for (int i = 0; i < 512; i++) {
        if (pml4[i].P) {
            if ((uint64_t)&pml4[i] > highest_addr) {
                highest_addr = (uint64_t)&pml4[i];
            }
            present_PML4_entries++;
            printf("PML4: Entry %#x is present, with value %#018llx and address %#08llx      \n\r", i, *(u64*)(pml4 + i), pml4[i].addr);
            pml4[i].addr = old_addr_to_new_map((uint64_t)pml4[i].addr);
            printf("PML4: Entry %#x is present, with value %#018llx and address %#08llx      \n\r", i, *(u64*)(pml4 + i), pml4[i].addr);
            struct PDP_entry *pdp = (struct PDP_entry*)(uint64_t)(pml4[i].addr << 12);// - (1 << 12);
            for (int j = 0; j < 512; j++) {
                if (pdp[j].P) {
                    if ((uint64_t)&pdp[j] > highest_addr) {
                        highest_addr = (uint64_t)&pdp[j];
                    }
                    present_PDP_entries++;
                    pdp[j].addr = old_addr_to_new_map((uint64_t)pdp[j].addr);
                    //printf("PDP: Entry %#x is present, with value %#016llx and address %#08llx\n\r", i, *(u64*)(pdp + i), pdp[i].addr);
                    struct PD_entry_2M *pd = (struct PD_entry_2M*)(uint64_t)(pdp[j].addr << 12);// - (1 << 12);
                    for (int k = 0; k < 512; k++) {
                        if (pd[k].P) {
                            if ((uint64_t)&pd[k] > highest_addr) {
                                highest_addr = (uint64_t)&pd[k];
                            }
                            present_PD_entries++;
                            if (pd[k].PS) {
                                present_PD_entries_2++;
                            } else {
                                struct PD_entry_4K *pd_4K = (struct PD_entry_4K*)(uint64_t)(pdp[j].addr << 12);// - (1 << 12);
                                //pd_4K->addr = old_addr_to_new_map(pd_4K->addr);
                                printf("Not PS bit set indexes: %#x, %#x, %#x   \n\r", i, j, k);
                                struct PT_entry *pt = (struct PT_entry*)(uint64_t)(pd_4K[k].addr << 12);// - (1 << 12);
                                printf("pt page addr: %#018llx    \n\r", pt);
                                for (int l = 0; l < 512; l++) {
                                    if (pt[l].P) {
                                        if ((uint64_t)&pt[l] > highest_addr) {
                                            //highest_addr = &pt[l];
                                            if (l == 511) {
                                                printf("Highest PT entry addr: %#018llx    \n\r", pd[l].addr);
                                            }
                                        }
                                        if (!present_PT_entries) {
                                            printf("First PT entry: %#018llx      \n\r", pt[l].addr);
                                            printf("Second PT entry: %#018llx      \n\r", pt[l+1].addr);
                                        }
                                        present_PT_entries++;
                                    } else if (!found_non_present_pt) {
                                        found_non_present_pt = 1;
                                        printf("First not present PT entry: index %#x, pd %#x, pdp %#x, pml4 %#x     \n\r", l, k, j, i);
                                    }
                                    if (present_PT_entries == 0x400 && !found_last_pt) {
                                        //uint64_t curr_addr = OFFSETS_TO_ADDR(i, j, k, l);
                                        printf("%#x, %#x, %#x, %#x, %#018llx, %#018llx, %#018llx     \n\r", i, j, k, l, pt[l].addr, &(pt[l]), 0);
                                        found_last_pt = 1;
                                    }
                                }
                            }
                            if (present_PD_entries == 0x8000 && !found_last_pd) {
                                found_last_pd = 1;
                                //uint64_t curr_addr = OFFSETS_TO_ADDR(i, j, k, 0);
                                printf("%#x, %#x, %#x, %#018llx, %#018llx, %#018llx     \n\r", i, j, k, pd[k].addr, &(pd[k]), 0);
                            }
                        }
                    }
                }
            }
        }
    }
    printf("present_PML4_entries: %#010llx   \n\r", present_PML4_entries);
    printf("present_PDP_entries: %#010llx    \n\r", present_PDP_entries);
    printf("present_PD_entries: %#010llx     \n\r", present_PD_entries);
    printf("present_PT_entries: %#010llx     \n\r", present_PT_entries);
    printf("Present pd entries with PS bit set: %#010llx         \n\r", present_PD_entries_2);
    printf("Highest addr: %018llx     \n\r", highest_addr);
    //*/

#undef old_addr_to_new_map

    void *virtual_ptr_1 = (void*)(uint64_t)0x1000400000;
    //void *virtual_ptr_2 = 0x0000400000;//0x1000800000;
    /*addr *virtual_addr_1 = (addr*)&virtual_ptr_1;
    addr *virtual_addr_2 = (addr*)&virtual_ptr_2;
    addr *pml4_addr = &pml4;*/

    int *buffer_1 = virtual_ptr_1;


    void *physical_addr = (void*)(uint64_t)0x1800000;
    int *buffer_2 = physical_addr;
    printf("Buffer 1: %#018llx, buffer 2: %#018llx         \n\r", buffer_1, buffer_2);
    // Intentionally broken line: page fault:
    //printf("Buffer 1 elem 0: %#018llx, buffer 2: %#018llx         \n\r", buffer_1[0], buffer_2[0]);

    struct PML4_entry *pml4_pml4 = pml4 + 0;
    struct PDP_entry *pml4_pdp = (struct PDP_entry*)(uint64_t)(pml4_pml4->addr << 12) + ((uint64_t)(PDP_MASK(pml4)) >> PDP_OFFSET);
    struct PD_entry_2M *pml4_pd = (struct PD_entry_2M*)(uint64_t)(pml4_pdp->addr << 12) + ((uint64_t)(PD_MASK(pml4)) >> PD_OFFSET);
    printf("%#018llx   ", *(uint64_t*)pml4_pml4);
    printf("%#018llx, %d\n\r", *(uint64_t*)pml4_pdp, pml4_pdp->R_W);
    printf("%#018llx, %d\n\r", *(uint64_t*)pml4_pd, pml4_pd->R_W);
    pml4_pd->R_W = 1;
    //struct PT_entry pml4_pt_entry = ((struct PT_entry*)(uint64_t)(((struct PD_entry_4K*)(uint64_t)(((struct PDP_entry*)(uint64_t)(pml4[pml4_addr->pml4_offset].addr << 12))[pml4_addr->pdp_offset].addr << 12))[pml4_addr->pd_offset].addr << 12))[pml4_addr->pt_offset];
    //printf("pml4 mapping present: %d, with .addr: %#018llx, at addr: %#018llx                             \n\r", pml4_pt_entry.P, pml4_pt_entry.addr << 12, &pml4_pt_entry);

    printf("pml4->R_W: %d\n\r", pml4->R_W);
    if (pml4[PML4_MASK(virtual_ptr_1) >> PML4_OFFSET].P) {
        printf("PML4 present! ");
        struct PDP_entry *pdp = (struct PDP_entry*) (uint64_t) (pml4[PML4_MASK(virtual_ptr_1) >> PML4_OFFSET].addr << 12);
        if (pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].P) {
            printf("PDP present! ");
            struct PD_entry_4K *pd = (struct PD_entry_4K*) (uint64_t) (pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].addr << 12);
            if (pd[PD_MASK(virtual_ptr_1) >> PD_OFFSET].P) {
                printf("PD present, page size bit: %d! ", pd->PS);
                struct PT_entry *pt = (struct PT_entry*) (uint64_t) (pd[PD_MASK(virtual_ptr_1) >> PD_OFFSET].addr << 12);
                if (pt[PT_MASK(virtual_ptr_1) >> PT_OFFSET].P) {
                    printf("PT present! ");
                    uint64_t addr = pt[PT_MASK(virtual_ptr_1) >> PT_OFFSET].addr << 12;
                    printf("%#x", addr);
                } else {
                    printf("PT not present, defining! ");
                    struct PT_entry *pte = &pt[PT_MASK(virtual_ptr_1) >> PT_OFFSET];
                    pte->P = 1;
                    pte->R_W = 1;
                    pte->U_S = 1;
                    pte->PWT = 0;
                    pte->PCD = 0;
                    pte->A = 0;
                    pte->D = 0;
                    pte->G = 0;
                    pte->PAT = 0;
                    pte->NX = 1;
                    pte->addr = (uint64_t)physical_addr;
                }
            } else {
                struct PD_entry_2M *pde = (struct PD_entry_2M *)&pd[PD_MASK(virtual_ptr_1) >> PD_OFFSET];
                pde->P = 1;
                pde->R_W = 1;
                pde->U_S = 1;
                pde->PWT = 0;
                pde->PCD = 0;
                pde->A = 0;
                pde->D = 0;
                pde->G = 0;
                pde->PAT = 0;
                pde->NX = 1;
                pde->addr = ((uint64_t)physical_addr >> 21);
            }
        } else {
            printf("PDP not present, filling! ");
            /*pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].P = 1;
            pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].R_W = 1;
            pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].U_S = 1;
            pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].PWT = 0;
            pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].PCD = 0;
            pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].A = 0;
            pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].PS = 0;
            pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].addr = 0x7c43;
            pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].NX = 0;*/
            struct PDP_entry *pdpe = pdp + (PDP_MASK(virtual_ptr_1) >> PDP_OFFSET);
            pdpe->P = 1;
            pdpe->R_W = 1;
            pdpe->U_S = 1;
            pdpe->PWT = 0;
            pdpe->PCD = 0;
            pdpe->A = 0;
            pdpe->PS = 0;
            pdpe->addr = 0x1444;
            pdpe->NX = 0;

            printf(" PD not present");
            //struct PD_entry_4K *pd = (struct PD_entry_4K*) (uint64_t) ;
            char *pd_4k_page = (char*)(uint64_t)(pdp[PDP_MASK(virtual_ptr_1) >> PDP_OFFSET].addr << 12);
            //printf(" %#018llx, %#018llx", pd, pd_4k_page);
            for (uint64_t i = 0; i < 4096; i++) {
                pd_4k_page[i] = 0;
            }
            printf(", filling!");
            struct PD_entry_2M *pde = &(((struct PD_entry_2M*)pd_4k_page)[PD_MASK(virtual_ptr_1) >> PD_OFFSET]);
            pde->P = 1;
            pde->R_W = 1;
            pde->U_S = 1;
            pde->PWT = 0;
            pde->PCD = 0;
            pde->A = 0;
            pde->D = 0;
            pde->G = 0;
            pde->PAT = 0;
            pde->NX = 1;
            pde->PS = 1;
            pde->addr = ((uint64_t)physical_addr >> 21);
        }
    }
    printf("\n\r");
/*
    if (pml4[PML4_MASK(virtual_ptr_2) >> PML4_OFFSET].P) {
        printf("PML4 present! ");
        struct PDP_entry *pdp = (struct PDP_entry*) (uint64_t) (pml4[PML4_MASK(virtual_ptr_2) >> PML4_OFFSET].addr << 12);
        if (pdp[PDP_MASK(virtual_ptr_2) >> PDP_OFFSET].P) {
            printf("PDP present! ");
            struct PD_entry_4K *pd = (struct PD_entry_4K*) (uint64_t) (pdp[PDP_MASK(virtual_ptr_2) >> PDP_OFFSET].addr << 12);
            if (pd[PD_MASK(virtual_ptr_2) >> PD_OFFSET].P) {
                printf("PD present, page size bit: %d! ", pd->PS);
                struct PT_entry *pt = (struct PT_entry*) (uint64_t) (pd[PD_MASK(virtual_ptr_2) >> PD_OFFSET].addr << 12);
                if (pt[PT_MASK(virtual_ptr_2) >> PT_OFFSET].P) {
                    printf("PT present! ");
                    uint64_t addr = pt[PT_MASK(virtual_ptr_2) >> PT_OFFSET].addr << 12;
                    printf("%#x", addr);
                } else {
                    printf("PT not present, defining! ");
                    struct PT_entry *pte = &pt[PT_MASK(virtual_ptr_2) >> PT_OFFSET];
                    pte->P = 1;
                    pte->R_W = 1;
                    pte->U_S = 1;
                    pte->PWT = 0;
                    pte->PCD = 0;
                    pte->A = 0;
                    pte->D = 0;
                    pte->G = 0;
                    pte->PAT = 0;
                    pte->NX = 1;
                    pte->addr = physical_addr;
                }
            } else {
                struct PD_entry_2M *pde = &pd[PD_MASK(virtual_ptr_2) >> PD_OFFSET];
                pde->P = 1;
                pde->R_W = 1;
                pde->U_S = 1;
                pde->PWT = 0;
                pde->PCD = 0;
                pde->A = 0;
                pde->D = 0;
                pde->G = 0;
                pde->PAT = 0;
                pde->NX = 1;
                pde->addr = ((uint64_t)physical_addr >> 21);
            }
        } else {
            struct PDP_entry *pdpe = &pdp[PDP_MASK(virtual_ptr_2) >> PDP_OFFSET];
            pdpe->P = 1;
            pdpe->R_W = 1;
            pdpe->U_S = 1;
            pdpe->PWT = 0;
            pdpe->PCD = 0;
            pdpe->A = 0;
            pdpe->PS = 0;
            pdpe->addr = 0x7c43;
            pdpe->NX = 0;

            struct PD_entry_4K *pd = (struct PD_entry_4K*) (uint64_t) (pdp[PDP_MASK(virtual_ptr_2) >> PDP_OFFSET].addr << 12);
            char *pd_4k_page = pd;
            for (uint64_t i = 0; i < 4096; i++) {
                pd_4k_page[i] = 0;
            }
            struct PD_entry_2M *pde = &pd[PD_MASK(virtual_ptr_2) >> PD_OFFSET];
            pde->P = 1;
            pde->R_W = 1;
            pde->U_S = 1;
            pde->PWT = 0;
            pde->PCD = 0;
            pde->A = 0;
            pde->D = 0;
            pde->G = 0;
            pde->PAT = 0;
            pde->NX = 1;
            pde->addr = ((uint64_t)physical_addr >> 21);
        }
    }
    printf("\n\r");
    */

    a = 0x1402000;
    __asm__ __volatile__ (
        "movq %0, %%cr3;" : : "r" (a)
    );

    printf("Buffer 1 elem 0: %#018llx, buffer 2: %#018llx         \n\r", buffer_1[0], buffer_2[0]);
    buffer_1[0] = 0x12345;
    printf("Buffer 1 elem 0: %#018llx, buffer 2: %#018llx         \n\r", buffer_1[0], buffer_2[0]);
    printf("Buffer 1 != Buffer 2: %s\n\r", buffer_1 != buffer_2 ? "true" : "false");

// kend:
    printf("Exiting!\n\r");
    return 0xBE2E76E43E;
}
