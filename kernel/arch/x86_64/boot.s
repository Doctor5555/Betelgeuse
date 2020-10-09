global _start

struc boot_table
    .mem_table_ptr resb 8
    .font_ptr resb 8
    .kernel_start_ptr resb 8
    .graphics_mode resb 40
endstruc

section .start

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