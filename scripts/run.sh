#!/bin/bash

# Change to the project root directory
cd "$(dirname "$0")"/.. || exit 1

# Build the OS
echo "Building oszoOS..."
make all

if [ $? -eq 0 ]; then
    echo "Build successful! ISO created at build/os-image.iso"
    echo "You can run it with: qemu-system-i386 -cdrom build/os-image.iso -m 256M -boot d"
else
    echo "Build failed!"
    exit 1
fi