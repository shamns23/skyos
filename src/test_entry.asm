; Simple test kernel entry
[bits 32]

; Multiboot header
section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002            ; magic number
    dd 0x00000000            ; flags: no special requirements
    dd -(0x1BADB002 + 0x00000000)  ; checksum

section .text
global _start
extern test_main

_start:
    ; Setup stack
    mov esp, stack_top
    
    ; Align stack to 16 bytes
    and esp, 0xFFFFFFF0
    
    ; Call test main
    call test_main
    
    ; Infinite loop
    jmp $

section .bss
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

; Add .note.GNU-stack section
section .note.GNU-stack noalloc noexec nowrite progbits