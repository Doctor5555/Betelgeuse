#include <kernel/boot_table.h>
#include <kernel/tty.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

void early_kmain(struct boot_table *boot_table) {
    terminal_init(boot_table);
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

u64 kmain(struct boot_table *boot_table) {
    /*if (terminal_init(boot_table)) {
        return 1;
    }*/
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
    ssp_test("Hello, Worlddsjkfghaskdhjfgasuiytg43iu57yqt34787qahgaeit784hit87f4yo87qg43gahwr87w4gi87orwaotw47tco27t4x8ba74c2684tcyr87antn24x3ntx87a3ywcknraw7tykvw37iayrxieugfau7wxt478aigweszt78hbawesgviydiawbszyiwtbhkgesivdo7GEUabk2qjuagiwefubkh2qiagwye78FIYwahtbj2qagiwyeFWahbjt2qwegiybht4kegiwySwabhkt42gisVYDhbrk23kfewgivydlstgeybhk2qktegwyiSDV&Tigeuwkt4bh2wgurls7dvtˆ©ukwbht42giuwe79tvdigeukbht2.37dtv9igeukbht4eguiktbhj4eguwisd;'opu[gpra[2]3otu2 ]wi4yvnOY@$^({*BYNVT&@$*NCN&T@{$98cY&#{N(*O&V[t48vwyt8v'w4tN*NYc984wty!");
    printf("Done ssp test 2 with Hello, World!!\n\r");
    //printf("Number of chars: %d Yay!\n\r", n_chars);
    /*
    for (u32 i = 0x2; i < 0x10; i += 0x1) {
        for (u32 j = 0x0; j < 0x10; i += 0x1) {
            terminal_putchar(i << 4 | j, i, j, 0x00FFFFFF, 0x00000000);
        }
    }*/
    //terminal_putchar('C', 1, 2, 0x00FFFFFF, 0x00000000);
    return 0xBE2E76E43E;
}