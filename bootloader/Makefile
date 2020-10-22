ARCH := x86_64

CC		    := clang
LD		    := /usr/local/opt/llvm/bin/lld-link

CFLAGS		:= -g -target $(ARCH)-pc-win32-coff -MMD -flto -ffreestanding -fno-stack-protector -fshort-wchar -Iinclude -std=c11 -mno-red-zone -Wall -Wextra
LDFLAGS		:= -subsystem:efi_application -nodefaultlib -dll
OBJS	    := arch/x86_64/uefi_boot.o arch/x86_64/memcpy.o arch/x86_64/guids.o arch/x86_64/uefi_print.o arch/x86_64/uefi_file.o arch/x86_64/uefi_mem.o arch/x86_64/uefi_tty.o

DESTDIR     := ../bin/iso
BOOTDIR     := /efi/boot

OVMF_URL	:= https://dl.bintray.com/no92/vineyard-binary/OVMF.fd
OVMF_BIN	:= OVMF.fd
OVMF		:= ../bin/$(OVMF_BIN)

.PHONY: all clean install install-efi

all: bootx64.efi

bootx64.efi: $(OBJS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -entry:efi_main $^ -out:$@

%.o: %.c
	$(CC) $(CFLAGS) $(BOOT_CFLAGS) -c $< -o $@

$(OVMF):
	mkdir -p ../bin
	wget $(OVMF_URL) -O $(OVMF) -qq

clean:
	rm -f bootx64.efi bootx64.lib
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: $(OVMF) install-efi

install-efi: bootx64.efi
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp bootx64.efi $(DESTDIR)$(BOOTDIR)
