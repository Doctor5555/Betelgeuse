DEFAULT_HOST!=../default-host.sh
HOST?=$(DEFAULT_HOST)
HOSTARCH?=../target-triplet-to-arch.sh $(HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=../sysroot
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include
RESDIR?=/resources

CFLAGS   := $(CFLAGS) -ffreestanding -Wall -Wextra -fstack-protector
CPPFLAGS := $(CPPFLAGS) -D__is_kernel -Iinclude -I../bootloder/include
LDFLAGS  := $(LDFLAGS)
LIBS     := $(LIBS) -nostdlib -L$(DESTDIR)$(PREFIX)/lib -lk -lgcc

ARCHDIR:=arch/x86_64

include $(ARCHDIR)/make.config

CFLAGS   := $(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS  := $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS     := $(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
kernel/kernel.o \
kernel/panic.o

CRTI_OBJ=crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=crtn.o

OBJS=\
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \


LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \
$(LIBS) \

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .asm

all: betelgeuse.kernel

betelgeuse.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.s.o:
	nasm -felf64 $< -o $@

clean:
	rm -f betelgeuse.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	mkdir -p $(DESTDIR)$(BOOTDIR)$(RESDIR)
	cp -R -p include/. $(DESTDIR)$(INCLUDEDIR)/.
	cp -R -p resources/. $(DESTDIR)$(BOOTDIR)$(RESDIR)/.
	#cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.
	#cp -R --preserve=timestamps resources/. $(DESTDIR)$(BOOTDIR)$(RESDIR)/.

install-kernel: betelgeuse.kernel
	echo $(DESTDIR)$(BOOTDIR)
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp betelgeuse.kernel $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
