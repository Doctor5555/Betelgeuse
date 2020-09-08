#include <kernel/boot_table.h>
#include <kernel/tty.h>
#include <types.h>

u64 kmain(struct boot_table *boot_table) {
    if (terminal_init(boot_table)) {
        return 1;
    }
    return 0xBE2E76E43E;
}