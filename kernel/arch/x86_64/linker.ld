ENTRY(_start)

KERNEL_VMA = 0xFFFFFFFF80000000;

SECTIONS
{
    . = KERNEL_VMA;

    __kernel_start = .;

    .text :
    {
        _code = .;
        *(.text)
        . = ALIGN(4096);
    }

   .data :
   {
        _data = .;
        *(.data)
        . = ALIGN(4096);
   }

    .rodata :
    {
		_rodata = .;
		*(.rodata*)
		. = ALIGN(4096);
	}

   .eh_frame :
   {
       _ehframe = .;
       *(.eh_frame)
        . = ALIGN(4096);
   }

   .bss :
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
   __kernel_end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}