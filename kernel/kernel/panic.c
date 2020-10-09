#include <kernel/panic.h>

#include <kernel/tty.h>

__attribute__ ((noreturn))
void kernel_panic(void) {
    terminal_writestring((unsigned char *)"\n\rkernel panic!\n\r");
    for(;;);
}