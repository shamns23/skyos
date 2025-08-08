# Makefile لبناء نظام تشغيل بسيط - SkyOS v4.1

# تعريف مجلد البناء
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso

C_SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir ${C_SOURCES:.c=.o}) kernel_entry.o)

# إنشاء مجلد البناء
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(ISO_DIR)/boot/grub

all: $(BUILD_DIR) os-image.iso

os-image.iso: $(BUILD_DIR)/kernel.elf $(ISO_DIR)/boot/grub/grub.cfg
	# نسخ ملف kernel.elf
	cp $(BUILD_DIR)/kernel.elf $(ISO_DIR)/boot/
	# إنشاء ملف ISO
	grub-mkrescue -o $(BUILD_DIR)/os-image.iso $(ISO_DIR)

$(ISO_DIR)/boot/grub/grub.cfg: $(BUILD_DIR)
	@echo 'set timeout=0' > $@
	@echo 'set default=0' >> $@
	@echo 'menuentry "SkyOS-v4.1" {' >> $@
	@echo '    multiboot /boot/kernel.elf' >> $@
	@echo '    boot' >> $@
	@echo '}' >> $@

# تجميع ملف boot.asm
$(BUILD_DIR)/boot.bin: src/boot.asm $(BUILD_DIR)
	nasm $< -f bin -o $@

# تجميع ملف kernel_entry.asm
$(BUILD_DIR)/kernel_entry.o: src/kernel_entry.asm $(BUILD_DIR)
	nasm $< -f elf32 -o $@

# تعريف متغيرات التجميع
CFLAGS = -m32 -ffreestanding -fno-pic -fno-pie -Wall -Wextra -Iinclude

# تجميع ملف kernel.c
$(BUILD_DIR)/kernel.o: src/kernel.c $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف fat32.c
$(BUILD_DIR)/fat32.o: src/fat32.c include/fat32.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف string_utils.c
$(BUILD_DIR)/string_utils.o: src/string_utils.c include/string_utils.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف command_handler.c
$(BUILD_DIR)/command_handler.o: src/command_handler.c include/command_handler.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف display.c
$(BUILD_DIR)/display.o: src/display.c include/display.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف io.c
$(BUILD_DIR)/io.o: src/io.c include/io.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف keyboard.c
$(BUILD_DIR)/keyboard.o: src/keyboard.c include/keyboard.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف filesystem.c
$(BUILD_DIR)/filesystem.o: src/filesystem.c include/filesystem.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف editor.c
$(BUILD_DIR)/editor.o: src/editor.c include/editor.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# تجميع ملف shell.c
$(BUILD_DIR)/shell.o: src/shell.c include/shell.h $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# ربط ملفات النواة
$(BUILD_DIR)/kernel.elf: $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/fat32.o $(BUILD_DIR)/string_utils.o $(BUILD_DIR)/display.o $(BUILD_DIR)/io.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/filesystem.o $(BUILD_DIR)/editor.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/command_handler.o
	ld -m elf_i386 -o $@ -T config/linker.ld $^ -nostdlib

# تشغيل نظام التشغيل باستخدام QEMU
run: all
	qemu-system-i386 -cdrom $(BUILD_DIR)/os-image.iso

# عرض معلومات البناء
info:
	@echo "=== SkyOS v4.1 Build Information ==="
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "ISO Directory: $(ISO_DIR)"
	@echo "Generated Files:"
	@echo "  - $(BUILD_DIR)/kernel.elf"
	@echo "  - $(BUILD_DIR)/os-image.iso"
	@echo "  - $(BUILD_DIR)/*.o (object files)"
	@echo "  - $(ISO_DIR)/boot/grub/grub.cfg"

# تنظيف الملفات المؤقتة
clean:
	rm -rf $(BUILD_DIR)
	@echo "✅ تم تنظيف مجلد البناء: $(BUILD_DIR)"

# تنظيف شامل (يشمل الملفات في المجلد الرئيسي)
clean-all: clean
	rm -rf *.bin *.o *.elf os-image.iso iso
	@echo "✅ تم تنظيف جميع الملفات المولدة"

# عرض محتويات مجلد البناء
list:
	@echo "=== محتويات مجلد البناء ==="
	@if [ -d "$(BUILD_DIR)" ]; then \
		find $(BUILD_DIR) -type f | sort; \
	else \
		echo "مجلد البناء غير موجود. قم بتشغيل 'make' أولاً."; \
	fi

.PHONY: all run clean clean-all info list