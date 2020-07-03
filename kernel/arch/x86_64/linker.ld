ENTRY(_start)

SECTIONS
{
	/* The kernel lives at 3GB + 1MB in virtual address space, */
	/* which is mapped to 1MB in physical address space. */
	. = 0xC0100000;

	/* Symbol that indicates the start address of the kernel */
	_kernel_start = .;
	.text ALIGN(4K) : AT (ADDR (.text) - 0xC0000000)
	{
		_code = .;
		*(.multiboot)
		*(.text)
	}

	.rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		_rodata = .;
		*(.rodata)
	}

	.data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
	{
		_data = .;
		*(.data)
	}

	.eh_frame ALIGN(4K) : AT (ADDR (.eh_frame) - 0xC0000000)
	{
		_ehframe = .;
		*(.eh_frame)
	}

	.bss ALIGN(4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		_bss = .;
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
	/* Symbol that indicates the end address of the kernel */
	_kernel_end = .;
}
