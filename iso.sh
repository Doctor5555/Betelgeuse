#!/bin/sh
set -e
. ./build.sh

mkdir -p iso
mkdir -p iso/boot
mkdir -p iso/boot/grub

cp sysroot/boot/betelgeuse.kernel iso/boot/betelgeuse.kernel
cat > iso/boot/grub/grub.cfg << EOF
menuentry "betelgeuse" {
    multiboot /boot/betelgeuse.kernel
}
EOF
grub-mkrescue -o betelgeuse.iso iso