global _start

section .text

_start:
    mov rdi, rcx
    extern kmain
    call kmain

    cli
.loop:
    jmp .loop

.size: equ $ - _start