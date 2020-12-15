#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#include <kernel/serial.h>
#endif

int putchar(int ic) {
#if defined(__is_libk)
    unsigned char c = (unsigned char) ic;
    terminal_write(&c, sizeof(c));
    serial_write(c); // @TEMPORARY This probably should not always happen.
#else
    // @TODO: Implement system calls
#endif
    return ic;
}
