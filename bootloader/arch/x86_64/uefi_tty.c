#include "uefi_tty.h"

/*
#include <types.h>
#include <kernel/psf.h>
#include <kernel/boot_table.h>
#include <string.h>*/
#include <stdint.h>
#include <psf.h>

static size_t char_width;
static size_t char_height;
static size_t screen_width;
static size_t screen_height;
static char  *font_buffer;
static uint32_t *framebuffer;
static size_t framebuffer_size;
static uint32_t fg_colour = 0x00DCDCDC;
static uint32_t bg_colour = 0x00000000;

static size_t terminal_column;
static size_t terminal_row;
static size_t TERMINAL_WIDTH; // Only set once in terminal_init, defines the number of characters across the terminal width
static size_t TERMINAL_HEIGHT; // Only set once in terminal_init, defines the number of lines in the terminal

static char hexchars[17] = "0123456789ABCDEF";
static char hex64outstr[17] = "0000000000000000";
static char hex8outstr[3] = "00";

static size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

int terminal_init(struct boot_table *boot_table) {
    struct psf2_header *psf = ((struct boot_table*)boot_table)->font_ptr;
    if (!PSF2_MAGIC_OK(psf->magic)) {
        return 1; // Magic is not OK now!
    }
    char_width = psf->width;
    char_height = psf->height;
    screen_width = boot_table->graphics_mode.width;
    screen_height = boot_table->graphics_mode.height;
    font_buffer = (unsigned char*)psf;
    framebuffer = boot_table->graphics_mode.framebuffer_base;
    framebuffer_size = boot_table->graphics_mode.framebuffer_size;
    TERMINAL_WIDTH = screen_width / char_width;
    TERMINAL_HEIGHT = screen_height / char_height;
    terminal_column = 0;
    terminal_row = 0;
    return 0;
}

void terminal_setcolour(uint32_t fg, uint32_t bg) {
    fg_colour = fg;
    bg_colour = bg;
}

void terminal_writestring(unsigned char *data) {
    terminal_write(data, strlen(data));
}

void terminal_write(unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        switch (data[i]) {
        case '\n':
            if (++terminal_row == TERMINAL_HEIGHT) {
                terminal_row = 0;
                terminal_column = 0;
            }
            break;
        case '\r':
            terminal_column = 0;
            break;
        case '\t':
            terminal_column += 4 - (terminal_column % 4);
            if (terminal_column == TERMINAL_WIDTH) {
                terminal_column = 0;
                if (++terminal_row == TERMINAL_HEIGHT) {
                    terminal_row = 0;
                }
            }
            break;
        default: {
            if (0x20 <= data[i] && data[i] < 0xDC) { // Printable range for current font. @TODO: make dynamic with unicode check?
                terminal_putchar(data[i]);
            }
            break;
        }
        }
    }
}

void terminal_putchar(unsigned char c) {
    terminal_putentryat(c, terminal_column, terminal_row, fg_colour, bg_colour);
    if (++terminal_column == TERMINAL_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == TERMINAL_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_putentryat(unsigned char c, size_t column, size_t row, uint32_t fg, uint32_t bg) {
    char *c_pixels = font_buffer + 
            ((struct psf2_header*)font_buffer)->headersize + 
            c * ((struct psf2_header*)font_buffer)->charsize;
    for (unsigned int i = 0; i < char_height; i++) {
        unsigned int mask = 1 << (char_width - 1);
        for (unsigned int j = 0; j < char_width; j++) {
            *(framebuffer + (row * 16 + i) * screen_width + column * 8 + j) = (c_pixels[i] & mask) ? fg : bg;
            mask >>= 1;
        }
    }
}

void terminal_newline() {
    if (++terminal_row == TERMINAL_HEIGHT) {
        terminal_row = 0;
        terminal_column = 0;
    }
}

void terminal_cursor(size_t x, size_t y) {
    terminal_column = x;
    terminal_row = y;
    if (terminal_column >= TERMINAL_WIDTH) {
        terminal_column = 0;
    }
    if (terminal_row >= TERMINAL_HEIGHT) {
        terminal_row = 0;
    }
}

void terminal_print_hex64(size_t val) {
    for (int8_t i = 0; i < 16; i++) {
        hex64outstr[15 - i] = hexchars[val & 0b1111];
        val = val >> 4;
    }
    terminal_write(hex64outstr, 17);
}

void terminal_print_hex8(unsigned char *msg, char val) {
    for (int8_t i = 1; i >= 0; i--) {
        hex8outstr[i] = hexchars[val & 0b1111];
        val = val >> 4;
    }
    terminal_writestring(msg);
    terminal_write(hex64outstr, 2);
}