CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

CFLAGS := $(CFLAGS) -I../kernel/include -I../bootloader/include

virtual_mem.out: virtual_mem/main.c ../kernel/kernel/memory.c
	gcc $(CFLAGS) -c -DTEST_VIRTUAL_MEM virtual_mem/main.c -o virtual_mem/main.o
	gcc $(CFLAGS) -c -DTEST_VIRTUAL_MEM ../kernel/kernel/memory.c -o virtual_mem/memory.o
	gcc $(CFLAGS) virtual_mem/main.o virtual_mem/memory.o -o virtual_mem.out