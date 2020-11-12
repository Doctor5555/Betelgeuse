#!/bin/sh

set -e
. ./config.sh

for PROJECT in $PROJECTS; do
    (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE clean)
done

rm -rf bin/iso
rm betelgeuse.img betelgeuse.iso