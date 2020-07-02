.set ALIGN,	1<<0
.set MEMINFO,	1<<1
.set FLAGS,	ALIGN | MEMINFO
.set MAGIC,	0x1BADB002
.set CHECKSUM,	-(MAGIC + FLAGS)


.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 4
stack_bottom:
	.skip 16384 # 16 KiB stack
stack_top:

.section .text
.global _start
.type _start, @function
_start:
	mov $stack_top, %esp

	/* @TODO: Floating point init, enable paging, etc*/

	call kernel_early_main

	call _init

	call kernel_main

	call _fini
	
	cli
_hang:	hlt
	jmp _hang

.size _start, . - _start
