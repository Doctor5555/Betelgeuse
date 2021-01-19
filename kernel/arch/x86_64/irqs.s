global irq_pic1
global irq_pic2
global load_idt

section .text

irq_pic1:
    push    r11
    push    r10
    push    r9
    push    r8
    push    rdi
    push    rsi
    push    rcx
    push    rdx
    push    rax

    mov rax, 0x20
    mov rdx, 0x20
    out dx, al

    pop     rax
    pop     rdx
    pop     rcx
    pop     rsi
    pop     rdi
    pop     r8
    pop     r9
    pop     r10
    pop     r11
    iret

irq_pic2:
    push    r11
    push    r10
    push    r9
    push    r8
    push    rdi
    push    rsi
    push    rcx
    push    rdx
    push    rax

    mov rax, 0x20
    mov rdx, 0x20
    out dx, al
    mov rax, 0xA0
    out dx, al
    
    pop     rax
    pop     rdx
    pop     rcx
    pop     rsi
    pop     rdi
    pop     r8
    pop     r9
    pop     r10
    pop     r11
    iret

idtr:
    dw 0
    dq 0

load_idt:
    mov [idtr + 2], rdi
    mov rax, rsi
    mov [idtr], ax
    lidt [idtr]
    sti
    ret
