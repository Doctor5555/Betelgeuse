#include <kernel/boot_table.h>
#include <kernel/tty.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include <efi_memory_descriptor.h>

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

    u64 a = 0;
    __asm__ __volatile__ (
        "movq %%cr3, %0;" : "=r" (a) :
    );
    printf("Contents of cr3: %#llx\n\r", a);
    printf("PML4 base addr: %#llx\n\r", a >> 12);

    u64 b = 0;
    __asm__ __volatile__ (
        "movq %%cr4, %0;" : "=r" (b) :
    );
    printf("Contents of cr4: %#llx\n\r", b);
    printf("PCIDE: %#llx\n\r", b & (0x1 << 17));

    printf("Here!\n\r");


    efi_memory_descriptor *memory_map = boot_table.mem_table_ptr;
    u64 descriptor_size = boot_table.mem_desc_size;
    u64 descriptor_count = boot_table.mem_desc_count;

    terminal_cursor(0, 0);

    printf("Memory descriptor count: %#018llx\n\r", descriptor_count);
    for (size_t i = 0; i < descriptor_count; i++) {
        efi_memory_descriptor *descriptor = boot_table.mem_table_ptr + i * descriptor_size;
        if (descriptor->Attribute & 0x8000000000000000) {
            //printf("Attribute: ");
            //printf("%010llx\n\ra\n\r", descriptor->Attribute);
            printf("index: %2d, attribute: 0x8%015llx, num pages: %#010llx, physical start: %#010llx, virt start: %#010llx, type: %#010x\n\r", 
                    i, descriptor->Attribute & 0x7fffffffffffffff, descriptor->NumberOfPages, descriptor->PhysicalStart, descriptor->VirtualStart, descriptor->Type);
            /*if (descriptor->Attribute & 0x8000000000000000) {
                printf("Attribute & 0x8000000000000000! val: %#010llx\n\r", descriptor->Attribute & 0x7fffffffffffffff);
            }*/
        } else
        printf("index: %2d, attribute: %#018llx, num pages: %#010llx, physical start: %#010llx, virt start: %#010llx, type: %#010x\n\r", 
                i, descriptor->Attribute, descriptor->NumberOfPages, descriptor->PhysicalStart, descriptor->VirtualStart, descriptor->Type);
    }

/*
    struct PML4_entry *pml4 = (struct PML4_entry*)a;

    u64 present_PML4_entries = 0;
    u64 present_PDP_entries = 0;
    u64 present_PD_entries = 0;
    u64 present_PT_entries = 0;
    for (int i = 0; i < 512; i++) {
        if (pml4[i].P) {
            present_PML4_entries++;
            //printf("PML4: Entry %#x is present, with value %#016llx and address %#08llx\n\r", i, *(u64*)(pml4 + i), pml4[i].addr);
            struct PDP_entry *pdp = (struct PDP_entry*) (pml4[i].addr << 12);
            for (int i = 0; i < 512; i++) {
                if (pdp[i].P) {
                    present_PDP_entries++;
                    //printf("PDP: Entry %#x is present, with value %#016llx and address %#08llx\n\r", i, *(u64*)(pdp + i), pdp[i].addr);
                    struct PD_entry *pd = (struct PD_entry*) (pdp[i].addr << 12);
                    for (int i = 0; i < 512; i++) {
                        if (pd[i].P) {
                            present_PD_entries++;
                            struct PT_entry *pt = (struct PT_entry*) (pd[i].addr << 12);
                            for (int i = 0; i < 512; i++) {
                                if (pt[i].P) {
                                    present_PT_entries++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    printf("present_PML4_entries: %#018llx\n\r", present_PML4_entries);
    printf("present_PDP_entries: %#018llx\n\r", present_PDP_entries);
    printf("present_PD_entries: %#018llx\n\r", present_PD_entries);
    printf("present_PT_entries: %#018llx\n\r", present_PT_entries);*/

    printf("Exiting!\n\r");
    return 0xBE2E76E43E;
}