#ifndef _TTY_H
#define _TTY_H

typedef unsigned long long size_t;

int terminal_init(void *font_struct);
void terminal_write(char *msg, size_t len);
void terminal_putchar(char c);

#endif /* _TTY_H */