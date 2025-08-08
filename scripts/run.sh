#!/bin/bash
set -e

# Change to project root directory
cd "$(dirname "$0")/.."

# Run make clean
if ! make clean; then
  echo "[ERROR] make clean failed!" >&2
  exit 1
fi

# Run make
if ! make; then
  echo "[ERROR] make failed!" >&2
  exit 1
fi

# Run QEMU
echo "Starting SkyOS in QEMU..."
qemu-system-i386 -cdrom build/os-image.iso -m 256 -nographic