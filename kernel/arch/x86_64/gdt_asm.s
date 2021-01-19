global set_gdt

section .text

gdtr:
    dw 0
    dq 0

set_gdt:
    mov [gdtr + 2], rdi
    mov rax, rsi
    mov [gdtr], ax
    lgdt [gdtr]
    ret