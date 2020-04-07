#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "types.h"

#if !defined(__i386__)
#error "This kernel requires an ix86-elf compiler at this time."
#endif

enum vga_colour {
	VGA_COLOUR_BLACK   = 0x0,
	VGA_COLOUR_BLUE    = 0x1,
	VGA_COLOUR_GREEN   = 0x2,
	VGA_COLOUR_CYAN    = 0x3,
	VGA_COLOUR_RED     = 0x4,
	VGA_COLOUR_MAGENTA = 0x5,
	VGA_COLOUR_BROWN   = 0x6,
	VGA_COLOUR_LIGHT_GREY    = 0x7,
	VGA_COLOUR_DARK_GREY     = 0x8,
	VGA_COLOUR_LIGHT_BLUE    = 0x9,
	VGA_COLOUR_LIGHT_GREEN   = 0xA,
	VGA_COLOUR_LIGHT_CYAN    = 0xB,
	VGA_COLOUR_LIGHT_RED     = 0xC,
	VGA_COLOUR_LIGHT_MAGENTA = 0xD,
	VGA_COLOUR_LIGHT_BROWN   = 0xE,
	VGA_COLOUR_WHITE   = 0xF
};

static inline u8 vga_entry_colour(enum vga_colour fg, enum vga_colour bg) {
	return fg | bg << 4;
}

static inline u16 vga_entry(unsigned char uc, u8 colour) {
	return (u16)uc | (u16)colour << 8;
}

size_t strlen(const char *str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
u8 terminal_colour;
u16 *terminal_buffer;

void terminal_init() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_colour = vga_entry_colour(VGA_COLOUR_LIGHT_GREY, VGA_COLOUR_BLACK);
	terminal_buffer = (u16*)0xB8000;
}

void terminal_set_colour(u8 colour) {
	terminal_colour = colour;
}

void terminal_putchar_at(char c, u8 colour, size_t x, size_t y) {
	terminal_buffer[y * VGA_WIDTH + x] = vga_entry(c, colour);
}

void terminal_putchar(char c) {
	terminal_putchar_at(c, terminal_colour, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			//TODO: scroll up
			terminal_row = 0;
		}
	}
}

void terminal_write(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char *data) {
	terminal_write(data, strlen(data));
}

void kernel_main() {
	terminal_init();

	terminal_writestring("Hello, World!\n");
}