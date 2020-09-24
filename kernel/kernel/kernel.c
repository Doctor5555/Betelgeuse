#include <kernel/boot_table.h>
#include <kernel/tty.h>
#include <kernel/memory.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include <efi_memory_descriptor.h>

struct boot_table boot_table;

void early_kmain(struct boot_table *boot_table_ptr) {
    boot_table = *boot_table_ptr;
    terminal_init(boot_table_ptr);
}

__attribute__ ((constructor)) void constructor_test() {
    terminal_writestring("Hello, Constructor World!\n\r");
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
    for (int i = 0; i < 6; i++) {
        terminal_write("Hello, World!\n\r", 15);
    }
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

    printf("Stuff!\n\r");
    //efi_memory_descriptor *memory_map = boot_table.mem_table_ptr;
    u64 descriptor_size = boot_table.mem_desc_size;
    u64 descriptor_count = boot_table.mem_desc_count;
    printf("Stuff 2!\n\r");

    terminal_cursor(0, 0);
    printf("Stuff 3!\n\r");

    u64 a = 0;
    __asm__ __volatile__ (
        "movq %%cr3, %0;" : "=r" (a) :
    );
    printf("Contents of cr3: %#llx\n\r", a);
    
    //*
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
    printf("CR4 address: %#018llx, and -(1 << 12): %#018llx\n\r", pml4, (u64)pml4 - (1 << 12));
    //pml4 = (u64)pml4 - (1 << 12);
    printf("CR4 address: %#018llx, and -(1 << 12): %#018llx\n\r", pml4, (u64)pml4 - (1 << 12));
    printf("Stuff 4!\n\r");

    //*
    u64 present_PML4_entries = 0;
    u64 present_PDP_entries = 0;
    u64 present_PD_entries = 0;
    u64 present_PT_entries = 0;
    char found_non_present_pt = 0;
    char found_last_pt = 0;
    for (int i = 0; i < 512; i++) {
        if (pml4[i].P) {
            present_PML4_entries++;
            printf("PML4: Entry %#x is present, with value %#018llx and address %#08llx      \n\r", i, *(u64*)(pml4 + i), pml4[i].addr);
            struct PDP_entry *pdp = (struct PDP_entry*) (pml4[i].addr << 12);// - (1 << 12);
            for (int j = 0; j < 512; j++) {
                if (pdp[j].P) {
                    present_PDP_entries++;
                    //printf("PDP: Entry %#x is present, with value %#016llx and address %#08llx\n\r", i, *(u64*)(pdp + i), pdp[i].addr);
                    struct PD_entry *pd = (struct PD_entry*) (pdp[j].addr << 12);// - (1 << 12);
                    for (int k = 0; k < 512; k++) {
                        if (pd[k].P) {
                            present_PD_entries++;
                            struct PT_entry *pt = (struct PT_entry*) (pd[k].addr << 12);// - (1 << 12);
                            for (int l = 0; l < 512; l++) {
                                if (pt[l].P) {
                                    if (!present_PT_entries) {
                                        printf("First PT entry: %#018llx      \n\r", pt[l].addr);
                                    }
                                    if (pt[l].addr == 5);
                                    present_PT_entries++;
                                } else if (!found_non_present_pt) {
                                    found_non_present_pt = 1;
                                    printf("First not present PT entry: index %#x, pd %#x, pdp %#x, pml4 %#x     \n\r", l, k, j, i);
                                }
                                if (present_PT_entries == 0x37e4 && !found_last_pt) {
                                    addr curr_addr;
                                    curr_addr.pml4_offset = i;
                                    curr_addr.pdp_offset = j;
                                    curr_addr.pd_offset = k;
                                    curr_addr.pt_offset = l;
                                    curr_addr.sign_extend = 0;
                                    curr_addr.page_offset = 0;
                                    printf("%#x, %#x, %#x, %#x, %#018llx, %#018llx, %#018llx                     \n\r", i, j, k, l, pt[l].addr, &(pt[l]), *(unsigned long long*)&curr_addr);
                                    found_last_pt = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    printf("present_PML4_entries: %#018llx   \n\r", present_PML4_entries);
    printf("present_PDP_entries: %#018llx    \n\r", present_PDP_entries);
    printf("present_PD_entries: %#018llx     \n\r", present_PD_entries);
    printf("present_PT_entries: %#018llx     \n\r", present_PT_entries);
    //*/

    void *virtual_ptr_1 = 0x100001000;
    void *virtual_ptr_2 = 0x100002000;
    addr *virtual_addr_1 = (addr*)&virtual_ptr_1;
    addr *virtual_addr_2 = (addr*)&virtual_ptr_2;
    addr *pml4_addr = &pml4;

    void *physical_addr = 0x1400000;

    struct PT_entry pml4_pt_entry = ((struct PT_entry*)(uint64_t)(((struct PD_entry*)(uint64_t)(((struct PDP_entry*)(uint64_t)(pml4[pml4_addr->pml4_offset].addr << 12))[pml4_addr->pdp_offset].addr << 12))[pml4_addr->pd_offset].addr << 12))[pml4_addr->pt_offset];
    printf("pml4 mapping present: %d, with .addr: %#018llx, at addr: %#018llx                             \n\r", pml4_pt_entry.P, pml4_pt_entry.addr << 12, &pml4_pt_entry);

    if (pml4[virtual_addr_1->pml4_offset].P) {
        printf("PML4 present! ");
        struct PDP_entry *pdp = (struct PDP_entry*) (uint64_t) (pml4[virtual_addr_1->pml4_offset].addr << 12);
        if (pdp[virtual_addr_1->pdp_offset].P) {
            printf("PDP present! ");
            struct PD_entry *pd = (struct PD_entry*) (uint64_t) (pdp[virtual_addr_1->pdp_offset].addr << 12);
            if (pd[virtual_addr_1->pd_offset].P) {
                printf("PD present! ");
                struct PT_entry *pt = (struct PT_entry*) (uint64_t) (pd[virtual_addr_1->pd_offset].addr << 12);
                if (pt[virtual_addr_1->pt_offset].P) {
                    printf("PT present! ");
                    uint64_t addr = pt[virtual_addr_1->pt_offset].addr << 12;
                    printf("%#x", addr);
                } else {
                    printf("PT not present, defining! ");
                    struct PT_entry pte = pt[virtual_addr_1->pt_offset];
                    pte.P = 1;
                    pte.R_W = 1;
                    pte.U_S = 1;
                    pte.PWT = 0;
                    pte.PCD = 0;
                    pte.A = 0;
                    pte.D = 0;
                    pte.G = 0;
                    pte.PAT = 0;
                    pte.NX = 1;
                    pte.addr = physical_addr;
                }
            }
        }
    }
    printf("\n\r");

    if (pml4[virtual_addr_2->pml4_offset].P) {
        printf("PML4 present! ");
        struct PDP_entry *pdp = (struct PDP_entry*) (uint64_t) (pml4[virtual_addr_2->pml4_offset].addr << 12);
        if (pdp[virtual_addr_2->pdp_offset].P) {
            printf("PDP present! ");
            struct PD_entry *pd = (struct PD_entry*) (uint64_t) (pdp[virtual_addr_2->pdp_offset].addr << 12);
            if (pd[virtual_addr_2->pd_offset].P) {
                printf("PD present! ");
                struct PT_entry *pt = (struct PT_entry*) (uint64_t) (pd[virtual_addr_2->pd_offset].addr << 12);
                if (pt[virtual_addr_2->pt_offset].P) {
                    printf("PT present! ");
                    uint64_t addr = pt[virtual_addr_2->pt_offset].addr << 12;
                    printf("%#x", addr);
                } else {
                    printf("PT not present, defining! ");
                    struct PT_entry pte = pt[virtual_addr_2->pt_offset];
                    pte.P = 1;
                    pte.R_W = 1;
                    pte.U_S = 1;
                    pte.PWT = 0;
                    pte.PCD = 0;
                    pte.A = 0;
                    pte.D = 0;
                    pte.G = 0;
                    pte.PAT = 0;
                    pte.NX = 1;
                    pte.addr = physical_addr;
                }
            }
        }
    }
    printf("\n\r");

    int *buffer_1 = virtual_addr_1;
    int *buffer_2 = virtual_addr_2;
    printf("Buffer 1: %#018llx, buffer 2: %#018llx         \n\r", buffer_1, buffer_2);
    printf("Buffer 1 elem 0: %#018llx, buffer 2: %#018llx         \n\r", buffer_1[0], buffer_2[0]);
    buffer_1[0] = 0x12345;
    printf("Buffer 1 elem 0: %#018llx, buffer 2: %#018llx         \n\r", buffer_1[0], buffer_2[0]);

    printf("Exiting!\n\r");
    return 0xBE2E76E43E;
}
