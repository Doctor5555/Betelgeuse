#ifndef _TTY_H
#define _TTY_H

typedef unsigned long long size_t;

int terminal_init(struct boot_table *boot_table);
void terminal_colour(u32 fg, u32 bg);
void terminal_write(char *msg, size_t len);
void terminal_putchar(char c, size_t x, size_t y, u32 fg, u32 bg);

#endif /* _TTY_H */