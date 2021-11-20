#include <kernel/interrupts.h>

#include <kernel/panic.h>
#include <kernel/serial.h>
#include <kernel/memory.h>

#include "apic.h"

#include <stdio.h>
//#include <cpuid.h>

struct idt_entry idt[256];

// Pointer to volatile structure
volatile struct apic_registers *apic_registers_pointer;

void print_interrupt_stack_frame(const char *type, struct InterruptStackFrame *frame) {
    printf("Exception: %s\n\rInterruptStackFrame {\n\r", type);
    printf("\tinstruction_pointer: %#018llx\n\r", frame->ip);
    printf("\tcode_segment: %#018llx\n\r", frame->cs);
    printf("\tcpu_flags: %#018llx\n\r", frame->rflags);
    printf("\tstack_pointer: %#018llx\n\r", frame->sp);
    printf("\tstack_segment: %#018llx\n\r", frame->ss);
    printf("}\n\r");
}

void __attribute__((interrupt)) breakpoint_handler(struct InterruptStackFrame *frame) {
    printf("BREAKPOINT CAUGHT!\n\r");
    print_interrupt_stack_frame("Breakpoint", frame);
}

void __attribute__((interrupt)) double_fault_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Double Fault", frame);
    kernel_panic();
}

void __attribute__((interrupt)) divide_by_zero_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Divide By Zero", frame);
    /* @TODO: Notify the scheduler to stop the current process because it has crashed. */
}

void __attribute__((interrupt)) overflow_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Overflow Exception", frame);
    /* @TODO: Notify the process of this event? */
}

void __attribute__((interrupt)) bound_range_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Bound Range Exception", frame);
    /* @TODO: Notify the process of this event? */
}

void __attribute__((interrupt)) invalid_opcode_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Invalid Opcode", frame);
    /* @TODO: Notify the process of this event? */
}

void __attribute__((interrupt)) device_not_available_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Device Not Available", frame);
    /* @TODO: Notify the process of this event? */
}

void __attribute__((interrupt)) invalid_tss_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Invalid TSS", frame);
    /* @TODO: What would cause this? I'm going to panic the kernel for now, since it is probably a fault on our side. */
    kernel_panic();
}

void __attribute__((interrupt)) segment_not_present_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Segment Not Present", frame);
    /* 
     * I don't think we should ever get here, because we are in long mode, 
     * but just in case, lets panic
     */
    kernel_panic();
}

void __attribute__((interrupt)) stack_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Stack Exception", frame);
    /* @TODO: Kill whatever process destroyed its own stack */
}

void __attribute__((interrupt)) general_protection_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("General Protection", frame);
    /* @TODO: Kill whatever process caused this. For now, just panic. */
    kernel_panic();
}

void __attribute__((interrupt)) page_fault_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Page Fault", frame);
    extern int8_t page_handle_page_fault();
    int8_t result = page_handle_page_fault();
    if (result != 0) {
        /*
         * @TODO: Don't just panic here. We should kill misbehaving processes,
         * and panic only if this happened in the kernel.
         */
        kernel_panic();
    }
    kernel_panic(); /* :PageFaults @TODO: When page_handle_page_fault is implemented, remove this.*/
}

void __attribute__((interrupt)) x87_floating_point_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("x87 Floating Point Exception", frame);
    /* @TODO: Kill whatever process caused this. */
}

void __attribute__((interrupt)) alignment_check_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Alignment Check", frame);
    /* @TODO: Kill whatever process caused this. */
}

void __attribute__((interrupt)) machine_check_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Machine Check", frame);
    /* @TODO: This is probably bad and should be displayed to the user as something. */
}

void __attribute__((interrupt)) simd_floating_point_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("SIMD Floating Point Exception", frame);
    /* @TODO: Kill whatever process caused this. */
}

void __attribute__((interrupt)) control_protection_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Control Protection Exception", frame);
    /* @TODO: Kill whatever process caused this. */
}

void __attribute__((interrupt)) hypervisor_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Hypervisor Injection Exception", frame);
    /* @TODO: Deal with this. */
}

void __attribute__((interrupt)) vmm_communication_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("VMM Communication Exception", frame);
    /* @TODO: Deal with this. */
}

void __attribute__((interrupt)) security_exception_handler(struct InterruptStackFrame *frame) {
    print_interrupt_stack_frame("Security Exception", frame);
    /* @TODO: Deal with this. */
}

void __attribute__((interrupt)) apic_spurious_interrupt(struct InterruptStackFrame *frame) {
    // Don't need to do anything here probably
    // @TODO: Do we want to count spurious interrupts?
}

void __attribute__((interrupt)) apic_local_interrupt_0(struct InterruptStackFrame *frame) {
    printf("lint0");
}

void __attribute__((interrupt)) apic_local_interrupt_1(struct InterruptStackFrame *frame) {
    printf("lint1");
}

void set_idt_entry(uint8_t idx, void *fn, uint16_t attributes, uint8_t ist) {
    uint64_t fn_addr = (uint64_t)fn;
    idt[idx].target_offset_15_0 = fn_addr & 0x0000ffff;
    idt[idx].target_offset_31_16 = (fn_addr >> 16) & 0x0000ffff;
    idt[idx].target_offset_63_32 = (fn_addr >> 32) & 0xffffffff;
    idt[idx].target_selector = 0x08; // @TODO: Don't hardcode target selector?
    idt[idx].type_attributes = attributes;
    idt[idx].interrupt_stack_table = ist & 0x07;
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
    
    set_idt_entry(0, &divide_by_zero_handler, 0x8e, 0);
    /* @TODO: Debug, NMI */
    set_idt_entry(3, &breakpoint_handler, 0x8e, 0);
    set_idt_entry(4, &overflow_exception_handler, 0x8e, 0);
    set_idt_entry(5, &bound_range_exception_handler, 0x8e, 0);
    set_idt_entry(6, &invalid_opcode_handler, 0x8e, 0);
    set_idt_entry(7, &device_not_available_handler, 0x8e, 0);
    set_idt_entry(8, &double_fault_handler, 0x8e, 1);
    /* Coprocessor-Segment-Overrun (unused/reserved) */
    set_idt_entry(10, &invalid_tss_handler, 0x8e, 0);
    set_idt_entry(11, &segment_not_present_handler, 0x8e, 0);
    set_idt_entry(12, &stack_exception_handler, 0x8e, 0);
    set_idt_entry(13, &general_protection_handler, 0x8e, 0);
    set_idt_entry(14, &page_fault_handler, 0x8e, 1);
    /* Reserved */
    set_idt_entry(16, &x87_floating_point_exception_handler, 0x8e, 0);
    set_idt_entry(17, &alignment_check_handler, 0x8e, 0);
    set_idt_entry(18, &machine_check_handler, 0x8e, 0);
    set_idt_entry(19, &simd_floating_point_exception_handler, 0x8e, 0);
    /* Reserved */
    set_idt_entry(20, &control_protection_exception_handler, 0x8e, 0);
    /* Reserved (22 - 27) */
    set_idt_entry(28, &hypervisor_exception_handler, 0x8e, 0);
    set_idt_entry(29, &vmm_communication_exception_handler, 0x8e, 0);
    set_idt_entry(30, &security_exception_handler, 0x8e, 0);
    /* Reserved */
    for (int i = 32; i < 40; i++)
        set_idt_entry(i, &irq_pic1, 0x8e, 0);
    for (int i = 40; i < 48; i++)
        set_idt_entry(i, &irq_pic2, 0x8e, 0);
    /* @TODO: External interrupts and software interrupts */
    set_idt_entry(0xFD, &apic_local_interrupt_0, 0x8e, 0);
    set_idt_entry(0xFE, &apic_local_interrupt_1, 0x8e, 0);
    set_idt_entry(0xFF, &apic_spurious_interrupt, 0x8e, 0);

    load_idt(idt, sizeof(idt));

    return 0;
}

/* disable_PIC() is derived largely from examples given on https://wiki.osdev.org/PIC */
void disable_PIC() {
    outb(PIC_1, ICW1_INIT | ICW1_ICW4);
    outb(PIC_2, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC_1_data, 0x20);
    outb(PIC_2_data, 0x20 + 0x08);
    io_wait();
    outb(PIC_1_data, 4);
    outb(PIC_2_data, 2);
    io_wait();
    outb(PIC_1_data, ICW4_8086);
    outb(PIC_2_data, ICW4_8086);
    io_wait();

    outb(PIC_1_data, 0xFF);
    outb(PIC_2_data, 0xFF);
}

int8_t install_apic() {
    {
        uint32_t edx, unused;
        __cpuid(1, unused, unused, unused, edx);
        if (!(edx & (1 << 9))) {
            return -1; // APIC not supported; This is not allowed.
        }
    }

    uint32_t high, low;
    __asm__ __volatile__ ("rdmsr" : "=a"(low), "=d"(high) : "c"(APIC_MSR));
    uint64_t apic_msr_val = (high << 32) | low;
    if (!(apic_msr_val | (1 << 11))) { // Set enable bit
        apic_msr_val |= (1 << 11);
        high = apic_msr_val >> 32;
        low = apic_msr_val & 0xFFFFFFFF;
        __asm__ __volatile__ ("wrmsr" : : "a"(low), "d"(high), "c"(APIC_MSR));
    }
    uint64_t apic_base_address = apic_msr_val & 0x000FFFFFFFFFF000;
    page_virtual_alloc(&apic_registers_pointer);
    int8_t map_result = page_map(apic_registers_pointer, apic_base_address);

    apic_registers_pointer->local_interrupt_vector_0 = 0xFD;
    apic_registers_pointer->local_interrupt_vector_1 = 0xFE;
    apic_registers_pointer->spurious_vector = 0x1FF;

    //printf("%#018llx\n\r", apic_base_address);
    //printf("%d\n\r", map_result);
    //printf("%#018llx, %#010llx, %#010llx\n\r", apic_registers_pointer, ((struct apic_registers*)(apic_base_address))->local_apic_version, ((uint32_t*)apic_base_address)[12]);
    
    return 0;
}
