#ifndef _TTY_H
#define _TTY_H

#include <kernel/boot_table.h>

typedef unsigned long long size_t;

int terminal_init(struct boot_table *boot_table);
void terminal_colour(unsigned int fg, unsigned int bg);
void terminal_write(unsigned char *msg, size_t len);
void terminal_putchar(unsigned char c, size_t x, size_t y, unsigned int fg, unsigned int bg);

#endif /* _TTY_H */