#include <kernel/serial.h>

static char hexchars[17] = "0123456789ABCDEF";
static char hex64outstr[17] = "0000000000000000";

/* From https://wiki.osdev.org/Serial_Ports */

void serial_init() {
   outb(COM1 + 1, 0x00);    // Disable all interrupts
   outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(COM1 + 1, 0x00);    //                  (hi byte)
   outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int is_transmit_empty() {
   return inb(COM1 + 5) & 0x20;
}
 
void serial_write(char a) {
   while (is_transmit_empty() == 0);
 
   outb(COM1,a);
}

void serial_write_string(char *a, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        serial_write(a[i]);
    }
}

void serial_write_hex64(uint64_t val) {
    for (int8_t i = 0; i < 16; i++) {
        hex64outstr[15 - i] = hexchars[val & 0b1111];
        val = val >> 4;
    }
    serial_write_string(hex64outstr, 17);
}