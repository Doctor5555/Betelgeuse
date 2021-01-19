global set_gdt

section .text

gdtr:
    dw 0
    dq 0

set_gdt:
    mov [gdtr + 2], rdi
    mov rax, rsi
    mov [gdtr], ax

    ;mov ax, 0x10
    lgdt [gdtr]

    ;mov ss, ax
    ret

.reload_segments:
    ;mov cs, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret
