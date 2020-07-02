#!/bin/sh
echo "$PATH" | grep -q "$HOME/opt/cross/bin" && echo "" ||export PATH="$HOME/opt/cross/bin:$PATH"

for PROJECT in $PROJECTS; do
    (cd $PROJECT && $MAKE clean)
done

rm -rf sysroot
rm -rf iso
rm -rf betelgeuse.iso
