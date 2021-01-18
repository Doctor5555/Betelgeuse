global _start

section .text

_start:
    extern early_kmain
    call early_kmain

    extern _init
    call _init

    extern kmain
    call kmain

    extern _fini
    call _fini

_kernel_halt:
    hlt
    jmp _kernel_halt

.size: equ $ - _start