#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "kernel/tty.h"

#include "types.h"
#include "vga.h"

static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;
static const u16 *VGA_MEMORY = (u16*) 0xC03FF000;

static size_t terminal_row;
static size_t terminal_column;
static u8 terminal_colour;
static u16 *terminal_buffer;

void terminal_initialize() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_colour = vga_entry_colour(VGA_COLOUR_LIGHT_GREY, VGA_COLOUR_BLACK);
	terminal_buffer = VGA_MEMORY;
	terminal_clear();
}

void terminal_set_colour(u8 colour) {
	terminal_colour = colour;
}

void terminal_putentryat(unsigned char data, u8 colour, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(data, colour);
}

void terminal_putchar(char c) {
    switch (c) {
	case 0x8:
		if (terminal_column > 0) {
			terminal_column--;
			terminal_putentryat(0, terminal_colour, terminal_column, terminal_row);
		}
		break;
	case '\t':
		terminal_column += (terminal_column + 4) % 4;
		break;
	case '\r':
		terminal_column = 0;
		break;
	case '\n':
		terminal_column = VGA_WIDTH;
		break;
	default:
		terminal_putentryat(c, terminal_colour, terminal_column, terminal_row);
	}
    if (++terminal_column >= VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row >= VGA_HEIGHT) {
			terminal_scroll(1 + terminal_row - VGA_HEIGHT);
			terminal_row -= 1;
		}
	}
}

void terminal_clear() {
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_colour);
		}
	}
}

void terminal_scroll(u8 lines) {
	u8 x = 0, base_y = 0, y = lines;
	while (y < VGA_HEIGHT) {
		for (; x < VGA_WIDTH; x++) {
			terminal_buffer[base_y * VGA_WIDTH + x] = terminal_buffer[y * VGA_WIDTH + x];
		}
		base_y++;
		y++;
		x = 0;
	}
	while (base_y < VGA_HEIGHT) {
		for (; x < VGA_WIDTH; x++) {
			terminal_buffer[base_y * VGA_WIDTH + x] = 0x0;
		}
		base_y++;
		x = 0;
	}
}

void terminal_write(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char *data) {
	terminal_write(data, strlen(data));
}

void terminal_writedigit(u8 digit) {
    if (digit < 10) {
        terminal_putchar(digit + 48);
    } else {
        terminal_writestring(" Unrecognised digit ");
    }
}

void terminal_writedenary(u32 num) {
    char digit_buffer[10];
    u8 i = 9;
    u8 rv = 0;
    do {
        digit_buffer[i] = num % 10;
        num /= 10;
        i--;
    } while (num > 0);
    rv = 9 - i;
    for (i++; i < 10; i++) {
        terminal_putchar(digit_buffer[i] + 48);
    }
    return rv;
}
