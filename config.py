PROJECTS = {"root": ".", "libc": "libc", "bootloader": "bootloader", "kernel": "kernel"}

GLOBAL_DEFINES = {
    "MAKE": "make",
    "PREFIX": "/sysres",
    "EXEC_PREFIX": "$(PREFIX)",
    "BOOTDIR": "/boot",
    "LIBDIR": "$(EXEC_PREFIX)/lib",
    "INCLUDEDIR": "$(PREFIX)/include",
    "RESDIR": "/resources",
}

ROOT_DEFINES = {
    "EMU": "qemu-system-x86_64",
    "MKGPT": "mkgpt",
    # "~/dev/Archive/mkgpt/mkgpt"
    "MKISO": "xorriso",
    "EMUFLAGS": "-drive if=pflash,format=raw,file=bin/OVMF.fd -drive format=raw,file=fat:rw:bin/iso -M accel=tcg -net none -serial stdio -m 128M -d int --no-reboot --no-shutdown",
    "EMUHDFLAGS": "-L bin/ -bios OVMF.fd -hda betelgeuse.bin",
    "EMUISOFLAGS": "-L bin/ -bios OVMF.fd -cdrom betelgeuse.iso",
}

BOOTLOADER_DEFINES = {
    "ARCH": "x86_64",
    "CC": "clang",
    "LD": "/usr/local/opt/llvm/bin/lld-link",
    "CFLAGS": "-O2 -g -target $(ARCH)-pc-win32-coff -MMD -flto -ffreestanding -fno-stack-protector -fshort-wchar -Iinclude -std=c11 -mno-red-zone -Wall -Wextra",
    "LDFLAGS": "-subsystem:efi_application -nodefaultlib -dll -entry:efi_main",
}

KERNEL_LIBC_DEFINES = {
    "HOST": "x86_64-elf",
    "HOSTARCH": "x86_64",
    "AR": "$(HOST)-ar",
    "AS": "nasm -felf64",
    "CC": "$(HOST)-gcc --sysroot=$(SYSROOT) -isystem=$(INCLUDEDIR)",
    "CFLAGS": "-O2 -g",
    "CPPFLAGS": "",
    "LDFLAGS": "",
    "LIBS": "",
    "SYSROOT": "$(shell pwd)/../sysroot",
}

DEFINE_MAP = {
    "root": ROOT_DEFINES,
    "libc": KERNEL_LIBC_DEFINES,
    "bootloader": BOOTLOADER_DEFINES,
    "kernel": KERNEL_LIBC_DEFINES,
}


def main():
    pass


if __name__ == "__main__":
    main()