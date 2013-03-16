#!/bin/bash
for f in $(find ../../src -iname "*.c" -or -iname "*.h" -or -iname "*.png" -or -iname "*.py" -or -iname "*.cl")
do
    v=${f#../../}
    echo ${v//\//-}
    ln -sf $f ${v//\//-}
done
