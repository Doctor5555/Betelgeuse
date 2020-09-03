CC		    := clang
GCC			:= ~/opt/cross/bin/x86_64-elf-gcc
LD		    := lld-link
GLD			:= ~/opt/cross/bin/x86_64-elf-ld
EMU		    := qemu-system-x86_64
MKGPT       := ~/dev/Archive/mkgpt/mkgpt
MKISO       := xorriso

ARCH := x86_64

BOOT_CFLAGS	:= -target $(ARCH)-pc-win32-coff -MMD -flto
CFLAGS		:= -ffreestanding -fno-stack-protector -fshort-wchar -I. -Iefi -std=c11 -mno-red-zone -Wall -Wextra
LDFLAGS		:= -subsystem:efi_application -nodefaultlib -dll
OBJS	    := boot/uefi_boot.o boot/memcpy.o boot/guids.o boot/uefi_print.o boot/uefi_file.o
BOOTFILE	:= bin/hdd/efi/boot/bootx64.efi

TESTOBJS	:= elf_test/test.o
TESTELF     := bin/hdd/test.out

OVMF_URL	:= https://dl.bintray.com/no92/vineyard-binary/OVMF.fd
OVMF_BIN	:= OVMF.fd
OVMF		:= bin/$(OVMF_BIN)

EMUFLAGS	:= -drive if=pflash,format=raw,file=bin/OVMF.fd -drive format=raw,file=fat:rw:bin/hdd -M accel=kvm:tcg -net none -serial stdio
EMUHDFLAGS  := -L bin/ -bios OVMF.fd -hda hdimage.bin
EMUISOFLAGS := -L bin/ -bios OVMF.fd -cdrom cdimage.iso

$(BOOTFILE): $(OBJS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -entry:efi_main $^ -out:$@

$(TESTELF): $(TESTOBJS)
	mkdir -p $(dir $@)
	$(GCC) $(CFLAGS) -nodefaultlibs -emain -nostartfiles $^ -o $@

%st.o: %st.c
	$(GCC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(BOOT_CFLAGS) -c $< -o $@
 
-include $(OBJ:.o=.d)

elf_test: $(TESTELF)

test: $(BOOTFILE) $(OVMF) $(TESTELF)
	$(EMU) $(EMUFLAGS)

hdtest: $(OVMF) hd
	$(EMU) $(EMUHDFLAGS)

hd: fatimg
	$(MKGPT) -o hdimage.bin --image-size 4096 --part fat.img --type system

isotest: iso
	$(EMU) $(EMUISOFLAGS)

iso: fatimg
	mkdir -p iso
	cp fat.img iso
	$(MKISO) -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso

usbtest: writeusb
	sudo qemu-system-x86_64 -bios bin/OVMF.fd -hdb /dev/sdb

writeusb: fatimg
	sudo dd if=fat.img of=/dev/sdb bs=1M

fatimg: $(BOOTFILE)
	dd if=/dev/zero of=fat.img bs=1k count=1440
	mformat -i fat.img -f 1440 ::
	mmd -i fat.img ::/EFI
	mmd -i fat.img ::/EFI/BOOT
	mcopy -i fat.img $(BOOTFILE) ::/EFI/BOOT
	mcopy -i fat.img bin/hdd/b.txt ::
	mcopy -i fat.img bin/hdd/a.txt ::
	mcopy -i fat.img bin/hdd/c.cfg ::
	mcopy -i fat.img bin/hdd/test.out ::

$(OVMF):
	mkdir -p bin
	wget $(OVMF_URL) -O $(OVMF) -qq

clean:
	rm -f $(BOOTFILE)
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f fat.img cdimage.iso hdimage.bin
 
.PHONY: test hdtest isotest usbtest hd iso writeusb fatimg elf_test clean