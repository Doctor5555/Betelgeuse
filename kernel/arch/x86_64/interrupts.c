#include <kernel/interrupts.h>

#include <stdio.h>

struct idt_entry idt[256];

void __attribute__((interrupt)) breakpoint_handler(struct InterruptStackFrame *frame) {
    printf("BREAKPOINT CAUGHT!\n\r");
    return;
}

int8_t install_interrupts() {
    extern int irq_pic1();
    extern int irq_pic2();
    extern int load_idt(uint64_t, uint64_t);

    for (uint64_t i = 0; i < 256; i++)
    {
        idt[i].interrupt_stack_table = 0;
        idt[i].target_offset_15_0 = 0;
        idt[i].target_offset_31_16 = 0;
        idt[i].target_offset_63_32 = 0;
        idt[i].target_selector = 0;
        idt[i].type_attributes = 0;
        idt[i]._reserved = 0;
    }
    
    idt[3].target_offset_15_0 = (uint64_t)&breakpoint_handler & 0x0000ffff;
    idt[3].target_offset_31_16 = ((uint64_t)&breakpoint_handler >> 16)& 0x0000ffff;
    idt[3].target_offset_63_32 = ((uint64_t)&breakpoint_handler >> 32)& 0xffffffff;
    idt[3].target_selector = 0x08;
    idt[3].type_attributes = 0x8e;
    idt[3].interrupt_stack_table = 0;

    load_idt(idt, sizeof(idt));

    return 0;
}