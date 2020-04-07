i686-elf-as boot.s -o bin/obj/boot.o
i686-elf-gcc -c kernel.c -o bin/obj/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T linker.ld -o bin/betelgeuse.bin -ffreestanding -O2 -nostdlib bin/obj/boot.o bin/obj/kernel.o -lgcc

typeset -i curr_ver=$(cat version)
((curr_ver=curr_ver+1))
echo $curr_ver > version 
