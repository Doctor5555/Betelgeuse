ENTRY(_start)

KERNEL_PMA = 0x0000000000080000;
KERNEL_VMA = 0xFFFFFFFF80000000;

SECTIONS
{
    . = KERNEL_PMA;

    .start : {
        _start_addr = .;
        *(.start_low)
        . = ALIGN(4096);
    }

    . += KERNEL_VMA;

    .text : AT(ADDR(.text) - KERNEL_VMA)
    {
        _code = .;
        *(.start_high)
        *(.text)
        . = ALIGN(4096);
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) {
		_rodata = .;
		*(.rodata*)
		. = ALIGN(4096);
	}

   .data : AT(ADDR(.data) - KERNEL_VMA)
   {
        _data = .;
        *(.data)
        . = ALIGN(4096);
   }

   .eh_frame : AT(ADDR(.eh_frame) - KERNEL_VMA)
   {
       _ehframe = .;
       *(.eh_frame)
        . = ALIGN(4096);
   }

   .bss : AT(ADDR(.bss) - KERNEL_VMA)
   {
       _bss = .;
       *(.bss)

       /*
        * You usually need to include generated COMMON symbols
        * under kernel BSS section or use gcc's -fno-common
        */

        *(COMMON)
       . = ALIGN(4096);
   }

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}