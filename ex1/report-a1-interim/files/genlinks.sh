#!/bin/bash
for f in $(find ../../src -iname "*.c" -or -iname "*.h")
do
    v=${f#../../}
    echo ${v//\//-}
    ln -sf $f ${v//\//-}
done
