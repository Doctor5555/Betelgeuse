#include <stdint.h>

#ifdef __is_libk

#include <kernel/panic.h>
#include <kernel/tty.h>

uintptr_t __stack_chk_guard = 0x7E1572135AEFC49D;
__attribute__ ((noreturn)) void 
__stack_chk_fail(void) {
    terminal_writestring("Stack smash protector triggered!");
    kernel_panic();
}

#else

uintptr_t __stack_chk_guard = 0x7E1572135AEFC49D;
__attribute__ ((noreturn)) void 
__stack_chk_fail(void) {
    abort("Stack smash protector triggered!");
}

#endif