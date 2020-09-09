#include <kernel/boot_table.h>
#include <kernel/tty.h>
#include <types.h>

u64 kmain(struct boot_table *boot_table) {
    if (terminal_init(boot_table)) {
        return 1;
    }
    u32 fg = 0xFFFFFF;
    u32 bg = 0;
    terminal_colour(fg, bg);
    for (int i = 0; i < 6; i++) {
        terminal_write("Hello, World!\n\r", 15);
    }
    for (unsigned char i = 0x20; i < 0x80; i++) {
        terminal_write(&i, 1);
    }
    /*
    for (u32 i = 0x2; i < 0x10; i += 0x1) {
        for (u32 j = 0x0; j < 0x10; i += 0x1) {
            terminal_putchar(i << 4 | j, i, j, 0x00FFFFFF, 0x00000000);
        }
    }*/
    //terminal_putchar('C', 1, 2, 0x00FFFFFF, 0x00000000);
    return 0xBE2E76E43E;
}