/*#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "types.h"
#include "keyboard.h"

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

void terminal_clear() {
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_colour);
		}
	}
}
*/
/*
void terminal_scroll(size_t lines) {
	if (lines < VGA_HEIGHT) {
		for (size_t y = 0; y < VGA_HEIGHT; y++) {
			for (size_t x = 0; x < VGA_WIDTH; x++) {
				terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + lines) * VGA_WIDTH + x];
			}
		}
	}
	else {
		terminal_clear();
	}
}
*/
/*
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

void terminal_init() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_colour = vga_entry_colour(VGA_COLOUR_LIGHT_GREY, VGA_COLOUR_BLACK);
	terminal_buffer = (u16*)0xC03FF000;
	terminal_clear();
}

void terminal_set_colour(u8 colour) {
	terminal_colour = colour;
}

void terminal_putchar_at(char c, u8 colour, size_t x, size_t y) {
	switch (c) {
	case 0x8:
		if (terminal_column > 0) {
			terminal_column--;
			terminal_buffer[y * VGA_WIDTH + x] = vga_entry(0, colour);
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
		terminal_buffer[y * VGA_WIDTH + x] = vga_entry(c, colour);
	}
}

void terminal_putchar(char c) {
	terminal_putchar_at(c, terminal_colour, terminal_column, terminal_row);
	terminal_column++;
	if (terminal_column >= VGA_WIDTH) {
		terminal_column = 0;
		terminal_row++;
		if (terminal_row >= VGA_HEIGHT) {
			terminal_scroll(1 + terminal_row - VGA_HEIGHT);
			terminal_row -= 1;
		}
	}
}

u16 terminal_write(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
	return size;
}

u16 terminal_writestring(const char *data) {
	return terminal_write(data, strlen(data));
}

void terminal_writedigit(u8 digit) {
    if (digit < 10) {
        terminal_putchar(digit + 48);
    } else {
        terminal_writestring(" Unrecognised digit ");
    }
}

u16 terminal_writedenary(u32 num) {
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

struct test_struct {
	u32 a;
	u32 b;
	char c;
} test_object;

__attribute__ ((constructor)) void foo(void)
{
	test_object.a = 12;
	test_object.b = 153;
	test_object.c = 'D';
	terminal_writestring("Hello, World from a constructor function!\n");
}

class TestClass {
	public:
	TestClass() {
		terminal_writestring("Hello, World from a class constructor!\n");
	}
};

TestClass test_class;

#ifdef __cplusplus
extern "C" {
#endif

void kernel_early_main() {
	terminal_init();
}

void kernel_main() {
	terminal_writestring("Welcome to Betelgeuse Operating System kernel v0.0.1!\n");
	//for (u8 i = 0; i < VGA_HEIGHT - 3; i++)
	//	terminal_writestring("A\n");
	kb_result r;
	while (true) {
		kb_read(&r);
		if (r.do_not_print)
			r.do_not_print = 0;
		else
			terminal_putchar(r.c);
		u8 x = terminal_column, y = terminal_row;
		terminal_column = VGA_WIDTH - 10;
		terminal_row = 0;
		terminal_writedenary(r.scancode);
		terminal_column = x;
		terminal_row = y;
	}
}

#ifdef __cplusplus
}
#endif
*/

#include <kernel/tty.h>

void kernel_early_main() {
	terminal_initialize();
}

void kernel_main() {
	terminal_writestring("Hello, World!");
}