#include <kernel/boot_table.h>
#include <kernel/tty.h>
#include <stdio.h>
#include <types.h>

void early_kmain(struct boot_table *boot_table) {
    terminal_init(boot_table);
}

__attribute__ ((constructor)) void constructor_test() {
    terminal_writestring("Hello, Constructor World!\n\r");
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