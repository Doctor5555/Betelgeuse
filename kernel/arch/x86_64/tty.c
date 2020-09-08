#include <kernel/tty.h>

#include <types.h>
#include <kernel/psf.h>
#include <kernel/boot_table.h>

static size_t char_width;
static size_t char_height;
static size_t screen_width;
static char  *char_array;
static u32 fg_colour = 0x00FFFFFF;
static u32 bg_colour = 0x00000000;

int terminal_init(struct boot_table *boot_table) {
    struct psf2_header *psf = ((struct boot_table*)boot_table)->font_ptr;
    if (!PSF2_MAGIC_OK(psf->magic)) {
        return 1; // Magic is not OK now!
    }
    char_width = psf->width;
    char_height = psf->height;
    screen_width = boot_table->graphics_mode.width;
    return 0;
}

void terminal_colour(u32 fg, u32 bg) {
    fg_colour = fg;
    bg_colour = bg;
}

void terminal_write(char *msg, size_t len) {

}

void terminal_putchar(char c, size_t x, size_t y, u32 fg, u32 bg) {

}
