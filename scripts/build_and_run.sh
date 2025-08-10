#!/bin/bash

# Script to build and run the OS
# يقوم هذا السكريبت ببناء النظام وتشغيله

echo "بدء بناء النظام..."
echo "Starting OS build..."

# Change to project root directory
cd "$(dirname "$0")/.."

# Build the OS
make all

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "✅ تم بناء النظام بنجاح!"
    echo "✅ Build successful!"
    echo ""
    echo "🚀 تشغيل النظام..."
    echo "🚀 Starting OS..."
    echo ""
    
    # Run QEMU with standard display (no VNC)
    qemu-system-i386 -cdrom build/os-image.iso -m 256M -boot d
else
    echo "❌ فشل في بناء النظام!"
    echo "❌ Build failed!"
    exit 1
fi