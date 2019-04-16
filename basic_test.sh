#!/bin/sh

rm fs/two.txt;
rm build/single_copy_basic;
gcc -o build/single_copy_basic src/single_copy_basic.c;
./build/single_copy_basic fs/one.txt fs/two.txt;
