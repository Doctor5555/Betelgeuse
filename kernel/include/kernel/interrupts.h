#ifndef _KERNEL_INTERRUPTS_H
#define _KERNEL_INTERRUPTS_H

#include <stdint.h>
#include <cpuid.h>

#if defined(ARCH_AMD64)

struct InterruptStackFrame {
    uint64_t cs;
    uint64_t ip;
    uint64_t rflags;
    uint64_t ss;
    uint64_t sp;
};

/*
 * 
 */
struct idt_entry {
    uint16_t target_offset_15_0;
    uint16_t target_selector;
    uint8_t interrupt_stack_table;
    uint8_t type_attributes;
    uint16_t target_offset_31_16;
    uint32_t target_offset_63_32;
    uint32_t _reserved;
};

#define PIC_1 0x20
#define PIC_1_data (PIC_1 + 1)
#define PIC_2 0xA0
#define PIC_2_data (PIC_2 + 1)

#define PIC_EOI 0x20
#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

#define APIC_MSR 0x001B

int8_t install_apic();

#elif defined(ARCH_AARCH64)

#endif

int8_t install_interrupts();

#endif /* _KERNEL_INTERRUPTS_H */