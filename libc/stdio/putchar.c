#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#endif

int putchar(int ic) {
#if defined(__is_libk)
    unsigned char c = (unsigned char) ic;
    terminal_write(&c, sizeof(c));
#else
    // @TODO: Implement system calls
#endif
    return ic;
}
