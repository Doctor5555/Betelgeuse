#include <kernel/tty.h>

#include <types.h>
#include <kernel/psf.h>
#include <kernel/boot_table.h>
#include <string.h>

static uint64_t char_width;
static uint64_t char_height;
static uint64_t screen_width;
static uint64_t screen_height;
static char  *font_buffer;
static u32 *framebuffer;
static uint64_t framebuffer_size;
static u32 fg_colour = 0x00FFFFFF;
static u32 bg_colour = 0x00000000;

static uint64_t terminal_column;
static uint64_t terminal_row;
static uint64_t TERMINAL_WIDTH; // Only set once in terminal_init, defines the number of characters across the terminal width
static uint64_t TERMINAL_HEIGHT; // Only set once in terminal_init, defines the number of lines in the terminal

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

void terminal_setcolour(u32 fg, u32 bg) {
    fg_colour = fg;
    bg_colour = bg;
}

void terminal_writestring(unsigned char *data) {
    terminal_write(data, strlen(data));
}

void terminal_write(unsigned char *data, uint64_t len) {
    for (uint64_t i = 0; i < len; i++) {
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

void terminal_putentryat(unsigned char c, uint64_t column, uint64_t row, u32 fg, u32 bg) {
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

void terminal_cursor(uint64_t x, uint64_t y) {
    terminal_column = x;
    terminal_row = y;
    if (terminal_column >= TERMINAL_WIDTH) {
        terminal_column = 0;
    }
    if (terminal_row >= TERMINAL_HEIGHT) {
        terminal_row = 0;
    }
}
