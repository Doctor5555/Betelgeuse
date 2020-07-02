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

.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096

.section .text
.global _start
.type _start, @function
_start:
	/* Physical address of boot_page_table1 */
	movl $(boot_page_table1 - 0xC0000000), %edi

	/* Map address 0 first */
	movl $0, %esi
	/* Map 1023 pages. Page 1024 will be the VGA buffer */
	movl $1023, %ecx

1:
	/* Only map the kernel */
	cmpl $(_kernel_start - 0xC0000000), %esi
	jl 2f
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge 3f

	/* Map physical addresses as present and writable.
	 * @TODO: Ensure .text and .rodata are mapped as non-writable,
	 * as this maps them as writable.
	 */
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	/* Size of a page is 4096 bytes */
	addl $4096, %esi
	/* An entry in boot_page_table1 is 4 bytes */
	addl $4, %edi
	/* Loop to next entry if we haven't yet finished */
	loop 1b

3:
	/* Map VGA video memory */
	movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

	/* Map the page table to 0x0000000 and 0xC0000000 */
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

	/* Set cr3 to boot_page_directory address */
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	/* Enable paging and the write-protect bit */
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	/* Jump to higher half */
	lea 4f, %ecx
	jmp *%ecx

	/* Paging setup complete */
4:
	/* Unmap identity mapping */
	movl $0, boot_page_directory + 0

	/* Reload crc3 to force a TLB flush so the changes to take effect. */
	movl %cr3, %ecx
	movl %ecx, %cr3

	/* Set up the stack */
	mov $stack_top, %esp

	/* @TODO: Floating point init, enable paging, etc*/

	call kernel_early_main

	/* Constructor functions and global object constructors */
	call _init

	call kernel_main

	/* Destructor functions and global object destructors */
	call _fini
	
	cli
_hang:	hlt
	jmp _hang

.size _start, . - _start
