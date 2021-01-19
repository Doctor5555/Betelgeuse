#include <stdint.h>

uint64_t gdt[5];

extern void set_gdt(uint64_t gdt, uint64_t size);

#define CODE_ENTRY_BASE 0x002f98ff0000ffff
#define DATA_ENTRY_BASE 0x002f90ff0000ffff

int8_t load_gdt() {
    gdt[0] = 0;
    gdt[1] = CODE_ENTRY_BASE;
    gdt[2] = DATA_ENTRY_BASE;
    gdt[3] = CODE_ENTRY_BASE | (3 << 45);
    gdt[4] = DATA_ENTRY_BASE | (3 << 45);
    set_gdt(gdt, sizeof(gdt));
    return 0;
}
