#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <bootloader/boot_table.h>

int terminal_init(struct boot_table *boot_table);
void terminal_setcolour(unsigned int fg, unsigned int bg);
void terminal_writestring(unsigned char *data);
void terminal_write(unsigned char *str, uint64_t len);
void terminal_putchar(unsigned char c);
void terminal_putentryat(unsigned char c, uint64_t column, uint64_t row, unsigned int fg, unsigned int bg);

void terminal_newline();
void terminal_cursor(uint64_t x, uint64_t y);

#endif /* _KERNEL_TTY_H */