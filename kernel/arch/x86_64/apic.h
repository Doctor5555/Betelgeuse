#ifndef _ARCH_AMD64_APIC_H
#define _ARCH_AMD64_APIC_H

#include <stdint.h>

#define ALIGN(n) __attribute__((aligned (n)))

struct apic_registers {
    uint32_t ALIGN(16) _reserved_0[8];

    uint32_t ALIGN(16) local_apic_id;
    uint32_t ALIGN(16) local_apic_version;

    uint32_t ALIGN(16) _reserved_1[16];

    uint32_t ALIGN(16) task_priority;
    uint32_t ALIGN(16) arbitration_priority;
    uint32_t ALIGN(16) processor_priority;
    uint32_t ALIGN(16) end_of_interrupt;
    uint32_t ALIGN(16) remote_read;
    uint32_t ALIGN(16) logical_destination;
    uint32_t ALIGN(16) destination_format;
    uint32_t ALIGN(16) spurious_vector;

    uint32_t ALIGN(16) in_service_0;
    uint32_t ALIGN(16) in_service_1;
    uint32_t ALIGN(16) in_service_2;
    uint32_t ALIGN(16) in_service_3;
    uint32_t ALIGN(16) in_service_4;
    uint32_t ALIGN(16) in_service_5;
    uint32_t ALIGN(16) in_service_6;
    uint32_t ALIGN(16) in_service_7;

    uint32_t ALIGN(16) trigger_mode_0;
    uint32_t ALIGN(16) trigger_mode_1;
    uint32_t ALIGN(16) trigger_mode_2;
    uint32_t ALIGN(16) trigger_mode_3;
    uint32_t ALIGN(16) trigger_mode_4;
    uint32_t ALIGN(16) trigger_mode_5;
    uint32_t ALIGN(16) trigger_mode_6;
    uint32_t ALIGN(16) trigger_mode_7;

    uint32_t ALIGN(16) interrupt_request_0;
    uint32_t ALIGN(16) interrupt_request_1;
    uint32_t ALIGN(16) interrupt_request_2;
    uint32_t ALIGN(16) interrupt_request_3;
    uint32_t ALIGN(16) interrupt_request_4;
    uint32_t ALIGN(16) interrupt_request_5;
    uint32_t ALIGN(16) interrupt_request_6;
    uint32_t ALIGN(16) interrupt_request_7;

    uint32_t ALIGN(16) error_status;

    uint32_t ALIGN(16) _reserved_2[28];

    uint32_t ALIGN(16) interrupt_command_low;
    uint32_t ALIGN(16) interrupt_command_high;

    uint32_t ALIGN(16) local_timer_vector;
    uint32_t ALIGN(16) local_thermal_vector;
    uint32_t ALIGN(16) local_perf_counter_vector;
    uint32_t ALIGN(16) local_interrupt_vector_0;
    uint32_t ALIGN(16) local_interrupt_vector_1;
    uint32_t ALIGN(16) error_vector;
    uint32_t ALIGN(16) timer_initial_count;
    uint32_t ALIGN(16) timer_current_count;

    uint32_t ALIGN(16) _reserved_3[16];

    uint32_t ALIGN(16) timer_divide_config;

    uint32_t ALIGN(16) _reserved_4[4];

    uint32_t ALIGN(16) apic_extended_feature;
    uint32_t ALIGN(16) apic_extended_control;
    uint32_t ALIGN(16) specific_end_of_interrupt;

    uint32_t ALIGN(16) _reserved_5[20];

    uint32_t ALIGN(16) interrupt_enable_0;
    uint32_t ALIGN(16) interrupt_enable_1;
    uint32_t ALIGN(16) interrupt_enable_2;
    uint32_t ALIGN(16) interrupt_enable_3;
    uint32_t ALIGN(16) interrupt_enable_4;
    uint32_t ALIGN(16) interrupt_enable_5;
    uint32_t ALIGN(16) interrupt_enable_6;
    uint32_t ALIGN(16) interrupt_enable_7;

    uint32_t ALIGN(16) extended_local_interrupt_vector_0;
    uint32_t ALIGN(16) extended_local_interrupt_vector_1;
    uint32_t ALIGN(16) extended_local_interrupt_vector_2;
    uint32_t ALIGN(16) extended_local_interrupt_vector_3;
};

struct apic_local_vector_table_entry {
    uint8_t vector;
    uint8_t options;
    uint16_t _reserved;
};

#endif /* _ARCH_AMD64_APIC_H */