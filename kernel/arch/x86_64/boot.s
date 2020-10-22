global _start

section .text

_start:
    mov rdi, rcx
    
    extern early_kmain
    call early_kmain

    extern _init
    call _init

    extern kmain
    call kmain

    extern _fini
    call _fini

    cli
.loop:
    jmp .loop

.size: equ $ - _start