#ifndef _KERNEL_TTY_H_GUARD
#define _KERNEL_TTY_H_GUARD

#include <stddef.h>
#include "types.h"

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char *data, size_t size);
void terminal_writestring(const char *data);
void terminal_writedenary(u32 num);

void terminal_scroll(u8 lines);
void terminal_clear();

#endif /* _TTY_H_GUARD */