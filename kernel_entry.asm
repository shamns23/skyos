; نقطة دخول النواة متوافقة مع Multiboot
[bits 32]

; Multiboot header
section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002            ; magic number
    dd 0x00000003            ; flags: page align + memory info
    dd -(0x1BADB002 + 0x00000003)  ; checksum
    dd 0                     ; header_addr
    dd 0                     ; load_addr
    dd 0                     ; load_end_addr
    dd 0                     ; bss_end_addr
    dd 0                     ; entry_addr
    dd 0                     ; mode_type
    dd 0                     ; width
    dd 0                     ; height
    dd 0                     ; depth

section .text
global _start
extern main
extern timer_handler

_start:
    ; إعداد المكدس
    mov esp, stack_top
    
    ; استدعاء دالة main في النواة
    call main
    
    ; حلقة لا نهائية
    jmp $

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

; Timer interrupt handler (IRQ0)
global irq0_handler
irq0_handler:
    pusha
    call timer_handler
    popa
    iretd

; إضافة .note.GNU-stack section لحل التحذير
section .note.GNU-stack noalloc noexec nowrite progbits