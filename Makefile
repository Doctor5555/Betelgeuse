# --------------------------------------------------------------------
SYSTEM_HEADER_PROJECTS := libc kernel
PROJECTS := libc kernel bootloader

MAKE := make

PREFIX      := /sysres
EXEC_PREFIX := $(PREFIX)
BOOTDIR     := /boot
LIBDIR      := $(EXEC_PREFIX)/lib
INCLUDEDIR  := $(PREFIX)/include
RESDIR      := /resources

EMU		    := qemu-system-x86_64
MKGPT       := ~/dev/Archive/mkgpt/mkgpt
MKISO 		:= xorriso

EMUFLAGS	:= -drive if=pflash,format=raw,file=bin/OVMF.fd -drive format=raw,file=fat:rw:bin/iso -M accel=tcg -net none -serial stdio -m 128M -d int --no-reboot --no-shutdown
EMUHDFLAGS  := -L bin/ -bios OVMF.fd -hda betelgeuse.bin
EMUISOFLAGS := -L bin/ -bios OVMF.fd -cdrom betelgeuse.iso
# --------------------------------------------------------------------

BOOTFILE := bin/iso/efi/boot/bootx64.efi

.PHONY: headers build iso test isotest usbtest writeusb hd hdtest fatimg clean $(PROJECTS:=.build) $(SYSTEM_HEADER_PROJECTS:=.headers) $(PROJECTS:=.clean)

$(PROJECTS:=.build): $(SYSTEM_HEADER_PROJECTS:=.headers)
	make -C $(subst .build,,$@) install

$(SYSTEM_HEADER_PROJECTS:=.headers):
	make -C $(subst .headers,,$@) install-headers

$(PROJECTS:=.clean):
	make -C $(subst .clean,,$@) clean

build: $(PROJECTS:=.build)
	mkdir -p bin/iso/boot
	cp -r sysroot/boot bin/iso/
	./inc_version.sh
	
clean: $(PROJECTS:=.clean)
	rm -rf bin/iso
	rm -f betelgeuse.img betelgeuse.iso

headers: $(PROJECTS:=.headers)

test: build $(OVMF) $(TESTELF)
	$(EMU) $(EMUFLAGS)

isotest: iso
	$(EMU) $(EMUISOFLAGS)

iso: fatimg
	mkdir -p iso
	cp betelgeuse.img iso
	$(MKISO) -as mkisofs -R -f -e betelgeuse.img -no-emul-boot -o betelgeuse.iso iso

hdtest: $(OVMF) hd
	$(EMU) $(EMUHDFLAGS)

hd: fatimg
	$(MKGPT) -o betelgeuse.bin --image-size 4096 --part betelgeuse.img --type system

usbtest: writeusb
	sudo qemu-system-x86_64 -bios bin/OVMF.fd -hdb /dev/sdb

writeusb: fatimg
	sudo dd if=betelgeuse.img of=/dev/sdb bs=1M

fatimg: build
	dd if=/dev/zero of=betelgeuse.img bs=1k count=1440
	mformat -i betelgeuse.img -f 1440 ::
	mmd -i betelgeuse.img ::/EFI
	mmd -i betelgeuse.img ::/EFI/BOOT
	mcopy -i betelgeuse.img $(BOOTFILE) ::/EFI/BOOT
	mcopy -i betelgeuse.img -s bin/iso/boot ::
#	mcopy -i betelgeuse.img bin/hdd/test.out ::
