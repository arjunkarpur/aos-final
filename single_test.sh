#!/bin/sh

# Copy single file (basic)
echo "SINGLE TEST BASIC"
rm fs/two_basic.txt;
rm build/single_copy_basic;
gcc -o build/single_copy_basic src/single_copy_basic.c;
./build/single_copy_basic fs/one.txt fs/two_basic.txt;

# Copy single file using aio
echo "SINGLE TEST AIO"
rm fs/two_aio.txt;
rm build/single_copy_aio;
gcc -o build/single_copy_aio src/single_copy_aio.c -laio;
./build/single_copy_aio fs/one.txt fs/two_aio.txt;
