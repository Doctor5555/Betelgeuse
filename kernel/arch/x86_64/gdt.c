#include <stdint.h>

struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

struct gdt_entry_t gdt[5];

extern void set_gdt(uint64_t gdt, uint64_t size);

int8_t load_gdt() {
    
    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xAF); // Code segment
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xAF); // Data segment
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xAF); // User mode code segment
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xAF); // User mode data segment

    set_gdt(gdt, sizeof(gdt) - 1);
    return 0;
}

void gdt_set_entry(int32_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[idx].base_low = (base & 0xFFFF);
    gdt[idx].base_mid = (base >> 16) & 0xFF;
    gdt[idx].base_high = (base >> 24) & 0xFF;

    gdt[idx].limit_low = (limit & 0xFFFF);
    gdt[idx].granularity = (limit >> 16) & 0x0F;

    gdt[idx].granularity |= gran & 0xF0;
    gdt[idx].access = access;
}