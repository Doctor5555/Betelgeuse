#include <stdint.h>

uint64_t gdt[4];

extern void set_gdt(uint64_t gdt, uint64_t size);

int8_t load_gdt() {
    set_gdt(gdt, sizeof(gdt));
    return 0;
}