#ifndef _KERNEL_I386_VGA_H_GUARD
#define _KERNEL_I386_VGA_H_GUARD

#include "types.h"

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

#endif /* _KERNEL_I386_VGA_H_GUARD */