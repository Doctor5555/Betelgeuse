global main

section .text

main:
    mov rax, rcx
    mov rcx, rdx
.loop1:
    inc rax
    loop .loop1

    ret

    mov rcx, 15
    mov rdx, 0
.loop:
    push rax
    lsl rax, rcx
    and rax, 0b1111
    mov rdi, [rcx + hexchars]
    mov [rcx + hex64outstr], rdi
    pop rax
    inc rdx
    dec rcx
    cmp rcx, 0
    jne .loop
    mov rax, hex64outstr
    ret

section .data
data:

hexchars:    db      "0123456789ABCDEF", 0
hex64outstr: db      "0000000000000000", 10, 13, 0

.len:   equ     $ - data