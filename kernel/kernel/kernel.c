#include <bootloader/boot_table.h>
#include <kernel/tty.h>
#include <kernel/memory.h>
#include <kernel/interrupts.h>
#include <kernel/serial.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include <efi_memory_descriptor.h>

struct boot_table boot_table;

uint64_t early_kmain(struct boot_table *boot_table_pointer) {
    boot_table = *boot_table_pointer;
    terminal_init(&boot_table);
    serial_init();
    terminal_setcolour(0xDCDCDC, 0x000000);
    terminal_writestring("Hello, World from early kmain!\n\r");
    return boot_table.graphics_mode.framebuffer_base;
}

__attribute__ ((constructor))
void constructor_test() {
    terminal_writestring("Hello, Constructor World!\n\r");
}

__attribute__ ((noinline))
int ssp_test(const char *test) {
    // The bug here is intentional. It is not a bug.
    int len = strlen(test);
    char dest[3];
    for (int i = 0; i < len; i++) {
        dest[i] = test[i];
    }
    return dest[2] & test[1];
}
/*
ssp_test("12");
printf("Done ssp test 1 with 12!\n\r");
// These two lines of code intentionally trigger the stack smash protector to test that it works.
ssp_test("Hello, Worlddsjkfghaskdhjfgasuiytg43iu57yqt34787qahgaeit784hit87f4yo87qg43gahwr87w4gi87orwaotw47tco27t4x8ba74c2684tcyr87antn24x3ntx87a3ywcknraw7tykvw37iayrxieugfau7wxt478aigweszt78hbawesgviydiawbszyiwtbhkgesivdo7GEUabk2qjuagiwefubkh2qiagwye78FIYwahtbj2qagiwyeFWahbjt2qwegiybht4kegiwySwabhkt42gisVYDhbrk23kfewgivydlstgeybhk2qktegwyiSDV&Tigeuwkt4bh2wgurls7dvtˆ©ukwbht42giuwe79tvdigeukbht2.37dtv9igeukbht4eguiktbhj4eguwisd;'opu[gpra[2]3otu2 ]wi4yvnOY@$^({*BYNVT&@$*NCN&T@{$98cY&#{N(*O&V[t48vwyt8v'w4tN*NYc984wty!");
printf("Done ssp test 2 with Hello, World!!\n\r");
*/

uint64_t kmain() {
    uint8_t result = page_allocator_init(&boot_table);
    printf("page_allocator_init result: %d\n\r", result);

    printf("Exiting!\n\r");
    return 0xBE2E76E43E;
}
