#ifndef _SERIAL_PORT_H
#define _SERIAL_PORT_H

#include <stdint.h>

uint8_t inb(uint16_t _port);

void outb(uint16_t _port, uint8_t val);

/* From https://wiki.osdev.org/Serial_Ports */

#define PORT 0x3f8   /* COM1 */
 
void serial_init();

int is_transmit_empty();
 
void serial_write(char a);

void serial_write_string(char *a, uint16_t len);

void serial_write_hex64(uint64_t val);

#endif /* _SERIAL_PORT_H */