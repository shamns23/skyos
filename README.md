# oszoOS v4.1

oszoOS is an advanced operating system written in C and Assembly, featuring a custom FAT32 file system implementation.

## Features

- **Custom Kernel**: Written in C with Assembly bootstrap
- **FAT32 File System**: Full implementation with file operations
- **GRUB Bootloader**: Standard bootloader configuration
- **Memory Management**: Basic memory allocation and management
- **Interrupt Handling**: IRQ and system call support

## Project Structure

```
├── src/              # Source code files
│   ├── kernel.c      # Main kernel implementation
│   ├── fat32.c       # FAT32 file system driver
│   ├── kernel_entry.asm # Assembly kernel entry point
│   └── boot.asm      # Boot sector code
├── include/          # Header files
│   └── fat32.h       # FAT32 header definitions
├── config/           # Configuration files
│   └── linker.ld     # Linker script
├── scripts/          # Utility scripts
│   └── run.sh        # Quick run script
├── build/            # Build output directory
├── Makefile          # Build configuration
└── README.md         # Project documentation
```

## Building

```bash
make clean
make all
```

## Running

```bash
# Using QEMU
qemu-system-i386 -cdrom build/os-image.iso -m 256

# Or use the run script
./run.sh
```

## Requirements

- GCC cross-compiler for i386
- NASM assembler
- GRUB tools
- QEMU (for testing)
- xorriso (for ISO creation)

## Development

This operating system is built from scratch and includes:

- Low-level hardware interaction
- Custom memory management
- File system implementation
- Interrupt and exception handling

## License

Open source project for educational purposes.