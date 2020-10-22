#ifndef _TTY_H
#define _TTY_H

#include <boot_table.h>

int terminal_init(struct boot_table *boot_table);
void terminal_setcolour(unsigned int fg, unsigned int bg);
void terminal_writestring(unsigned char *data);
void terminal_write(unsigned char *str, size_t len);
void terminal_putchar(unsigned char c);
void terminal_putentryat(unsigned char c, size_t column, size_t row, unsigned int fg, unsigned int bg);
void terminal_print_hex64(size_t val);
void print_hex8(unsigned char *msg, char val);

void terminal_newline();
void terminal_cursor(size_t x, size_t y);

#endif /* _TTY_H */