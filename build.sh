#!/bin/sh
#echo "$PATH" | grep -q "$HOME/opt/cross/bin" && echo "" || export PATH="$HOME/opt/cross/bin:$PATH"
set -e
. ./headers.sh

for PROJECT in $PROJECTS; do
    (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done

typeset -i curr_ver=$(cat version)
((curr_ver=curr_ver+1))
echo $curr_ver > version 
