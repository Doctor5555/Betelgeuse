global _start

section .text

_start:
    push rdi

    extern load_gdt
    call load_gdt

    pop rdi
    extern early_kmain
    call early_kmain

    extern _init
    call _init

    extern kmain
    call kmain

.halt:
    hlt
    jmp .halt

.size: equ $ - _start

