#include <kernel/boot_table.h>
#include <kernel/tty.h>
#include <kernel/paging.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include <efi_memory_descriptor.h>

struct boot_table boot_table;

uint64_t early_kmain(struct boot_table *boot_table_ptr) {
    boot_table = *boot_table_ptr;
    terminal_init(&boot_table);
    terminal_writestring("Hello, World from early kmain!\n\r");
    return boot_table.graphics_mode.framebuffer_base;
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

u64 kmain() {/*
    if (boot_table.kernel_start_ptr == 0) {
        printf("Kernel start ptr is zero! This should not be the case!\n\r");
    } else {
        printf("boot_table->kernel_start_ptr: %#018llx\n\r", boot_table.kernel_start_ptr);
    }*/

    //terminal_writestring("Hello, World!");

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
    printf("\n\rHello, Printf World 2!\n\r");
    printf("\n\rHello, Printf World %#05x!\n\r", 5000);

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

    efi_memory_descriptor *memory_map = boot_table.mem_table_ptr;
    u64 descriptor_size = boot_table.mem_desc_size;
    u64 descriptor_count = boot_table.mem_desc_count;

    terminal_cursor(0, 0);
    
    ///*
    printf("Memory descriptor count: %#018llx\n\r", descriptor_count);
    u64 page_count = 0;
    for (size_t i = 0; i < descriptor_count; i++) {
        efi_memory_descriptor *descriptor = boot_table.mem_table_ptr + i * descriptor_size;
        page_count += descriptor->number_of_pages;
        printf("index: %2d, attribute: %#018llx, num pages: %#010llx, physical start: %#010llx, virt start: %#010llx, type: %#010x\n\r", 
                i, descriptor->attribute, descriptor->number_of_pages, descriptor->physical_start, descriptor->virtual_start, descriptor->type);
    }

    printf("Total number of pages: %d, %#llx\n\r", page_count, page_count);
    //*/

    uint64_t *pml4 = boot_table.pml4_ptr;
    uint64_t pdp_count = 0;
    uint64_t pd_count = 0;
    for (int i = 0; i < 1; i++) {
        if (pml4[i] & PRESENT_FLAG) {
            uint64_t *pdp = pml4[i] & ADDR_MASK;
            for (int j = 0; j < 512; j++) {
                if (pdp[j] & PRESENT_FLAG) {
                    pdp_count++;
                    if (!(pdp[j] & PAGE_SIZE_FLAG)) {
                        uint64_t *pd = pdp[j] & ADDR_MASK;
                        for (int k = 0; k < 512; k++) {
                            if (pd[k] & PRESENT_FLAG) {
                                pd_count++;
                                if ((pd[k] & ADDR_MASK) != OFFSETS_TO_ADDR(i, j, k, 0)) {
                                    printf("pdp[%d] pd[%d] addr %#018llx != ADDR(%#x, %#x, %#x, 0) (%#018llx)\n\r", j, k, pd[k] & ADDR_MASK, i, j, k, OFFSETS_TO_ADDR(i, j, k, 0));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    printf("pdp_count: %#x (%d), pd_count: %#x (%d)\n\r", pdp_count, pdp_count, pd_count, pd_count);

// kend:
    printf("Exiting!\n\r");
    return 0xBE2E76E43E;
}
