#include <kernel/panic.h>

#include <kernel/tty.h>

static uint8_t panic_guard = 0;

__attribute__ ((noreturn))
void kernel_panic(void) {
    if (panic_guard)
        for(;;); // We have already panicked before, so either writestring or the debugger just panicked and we are broken.

    panic_guard++;
    terminal_writestring((unsigned char *)"\n\rkernel panic!\n\r");

#if _ENABLE_KERNEL_DEBUGGER
    #include <modules/static/debugger/debugger.h>
    execute_debugger();
#endif

    for(;;);
}
