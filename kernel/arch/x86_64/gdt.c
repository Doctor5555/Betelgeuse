#include <stdint.h>

struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

struct gdt_entry_long_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid_0;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_mid_1;
    uint32_t base_high;
    uint32_t mbz;
};

struct tss_t {
    uint32_t _reserved_0;
    uint32_t rsp[6];
    uint32_t _reserved_1[2];
    uint32_t ist[14];
    uint32_t _reserved_2[2];
    uint16_t _reserved_3;
    uint16_t iopb_offset;
};

struct gdt_entry_t gdt[7];
struct tss_t tss;

extern void set_gdt(uint64_t gdt, uint64_t size);

int8_t load_gdt() {
    
    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xAF); // Code segment
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xAF); // Data segment
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xAF); // User mode code segment
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xAF); // User mode data segment
    gdt_set_entry_long(5, &tss, sizeof(struct tss_t), 0x89, 0x40); // Task state segment

    set_gdt(gdt, sizeof(gdt) - 1);
    return 0;
}

void gdt_set_entry(int32_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[idx].base_low = (base & 0xFFFF);
    gdt[idx].base_mid = (base >> 16) & 0xFF;
    gdt[idx].base_high = (base >> 24) & 0xFF;

    gdt[idx].limit_low = (limit & 0xFFFF);
    gdt[idx].granularity = (limit >> 16) & 0x0F;

    gdt[idx].granularity |= gran & 0xF0;
    gdt[idx].access = access;
}

void gdt_set_entry(int32_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    struct gdt_entry_long_t *gdt_l = &gdt[idx];
    gdt_l->mbz = 0;

    gdt_l->base_low = (base & 0xFFFF);
    gdt_l->base_mid_0 = (base >> 16) & 0xFF;
    gdt_l->base_mid_1 = (base >> 24) & 0xFF;
    gdt_l->base_high = base >> 32;

    gdt_l->limit_low = (limit & 0xFFFF);
    gdt_l->granularity = (limit >> 16) & 0x0F;

    gdt_l->granularity |= gran & 0xF0;
    gdt_l->access = access;
}