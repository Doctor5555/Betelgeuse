global set_gdt
global set_tss

section .text

gdtr:
    dw 0
    dq 0

set_gdt:
    mov [gdtr + 2], rdi
    mov rax, rsi
    mov [gdtr], ax

    mov rdi, qword gdtr
    lgdt [rdi]

.reload_segments:
    mov rax, 0x10
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x8
    push .reload_cs
    o64 retf
.reload_cs:
    ret

set_tss:
    mov rax, rdi
    ltr ax
    ret