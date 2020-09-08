#!/bin/sh
#echo "$PATH" | grep -q "$HOME/opt/cross/bin" && echo "" ||export PATH="$HOME/opt/cross/bin:$PATH"
set -e
. ./config.sh

for PROJECT in $PROJECTS; do
    (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE clean)
done

rm -rf bin/iso
