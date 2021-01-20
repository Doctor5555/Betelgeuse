#include <kernel/interrupts.h>

#include <kernel/panic.h>

#include <stdio.h>

struct idt_entry idt[256];

void __attribute__((interrupt)) breakpoint_handler(struct InterruptStackFrame *frame) {
    printf("BREAKPOINT CAUGHT!\n\r");
}

void __attribute__((interrupt)) double_fault_handler(struct InterruptStackFrame *frame) {
    printf("Exception: Double Fault\n\rInterruptStackFrame {\n\r");
    printf("\tinstruction_pointer: %#018llx\n\r", frame->ip);
    printf("\tcode_segment: %#018llx\n\r", frame->cs);
    printf("\tcpu_flags: %#018llx\n\r", frame->rflags);
    printf("\tstack_pointer: %#018llx\n\r", frame->sp);
    printf("\tstack_segment: %#018llx\n\r", frame->ss);
    printf("}\n\r");
    kernel_panic();
}

void set_idt_entry(uint8_t idx, void *fn, uint16_t attributes) {
    uint64_t fn_addr = (uint64_t)fn;
    idt[idx].target_offset_15_0 = fn_addr & 0x0000ffff;
    idt[idx].target_offset_31_16 = (fn_addr >> 16) & 0x0000ffff;
    idt[idx].target_offset_63_32 = (fn_addr >> 32) & 0xffffffff;
    idt[idx].target_selector = 0x08;
    idt[idx].type_attributes = attributes;
    idt[idx].interrupt_stack_table = 0;
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
    
    set_idt_entry(3, &breakpoint_handler, 0x8e);
    set_idt_entry(8, &double_fault_handler, 0x8e);

    load_idt(idt, sizeof(idt));

    return 0;
}