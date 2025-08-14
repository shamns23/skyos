; نقطة دخول النواة متوافقة مع Multiboot
[bits 32]

; Multiboot header
section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002            ; magic number (required)
    dd 0x00000003            ; flags: page align modules and provide memory map
    dd -(0x1BADB002 + 0x00000003)  ; checksum (magic + flags + checksum = 0)

section .text
global _start
extern main
extern timer_handler

_start:
    ; إعداد المكدس
    mov esp, stack_top
    
    ; تأكد من أن المكدس محاذي على 16 بايت
    and esp, 0xFFFFFFF0
    
    ; استدعاء دالة main في النواة
    call main
    
    ; حلقة لا نهائية
    jmp $

; Timer interrupt handler (IRQ0)
global irq0_handler
irq0_handler:
    pusha
    call timer_handler
    popa
    iretd

section .bss
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

; إضافة .note.GNU-stack section لحل التحذير
section .note.GNU-stack noalloc noexec nowrite progbits