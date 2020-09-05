global _start

section .text

_start:
    xor rax, rax
    mov rax, 0xBE2E76E43E
    ret

.size: equ $ - _start