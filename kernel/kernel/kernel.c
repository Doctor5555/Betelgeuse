#include <kernel/tty.h>

void kernel_early_main() {
	terminal_initialize();
}

void kernel_main() {
	terminal_writestring("Welcome to Betelgeuse Operating System kernel v0.0.2!\n");
}