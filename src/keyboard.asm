; oszoOS Keyboard Driver - Assembly Implementation
; This file replaces keyboard.c with pure assembly

[BITS 32]
[GLOBAL kbd_wait]
[GLOBAL init_keyboard]
[GLOBAL get_char]
[GLOBAL kbd_flags]
[GLOBAL kbd_us]
[GLOBAL kbd_us_shift]

; Keyboard ports
KBD_DATA_PORT equ 0x60
KBD_STATUS_PORT equ 0x64
KBD_COMMAND_PORT equ 0x64

; Key flags
KEY_SHIFT equ 0x01
KEY_CTRL equ 0x02
KEY_ALT equ 0x04
KEY_CAPS equ 0x08
KEY_NUM_LOCK equ 0x10
KEY_SCROLL_LOCK equ 0x20

; Special key scancodes
KEY_ARROW_UP equ 0x48
KEY_ARROW_DOWN equ 0x50
KEY_ARROW_LEFT equ 0x4B
KEY_ARROW_RIGHT equ 0x4D
KEY_HOME equ 0x47
KEY_END equ 0x4F
KEY_PAGE_UP equ 0x49
KEY_PAGE_DOWN equ 0x51
KEY_INSERT equ 0x52
KEY_DELETE equ 0x53
KEY_F1 equ 0x3B
KEY_F2 equ 0x3C
KEY_F3 equ 0x3D
KEY_F4 equ 0x3E
KEY_F5 equ 0x3F
KEY_F6 equ 0x40
KEY_F7 equ 0x41
KEY_F8 equ 0x42
KEY_F9 equ 0x43
KEY_F10 equ 0x44
KEY_F11 equ 0x57
KEY_F12 equ 0x58

; Special key codes
ARROW_LEFT equ 0x80
ARROW_RIGHT equ 0x81
ARROW_UP equ 0x82
ARROW_DOWN equ 0x83
KEY_HOME_CODE equ 0x84
KEY_END_CODE equ 0x85
KEY_PGUP_CODE equ 0x86
KEY_PGDN_CODE equ 0x87
KEY_INS_CODE equ 0x88
KEY_DEL_CODE equ 0x89
KEY_ESC_CODE equ 0x1B
F1_CODE equ 0x90
F2_CODE equ 0x91
F3_CODE equ 0x92
F4_CODE equ 0x93
F5_CODE equ 0x94
F6_CODE equ 0x95
F7_CODE equ 0x96
F8_CODE equ 0x97
F9_CODE equ 0x98
F10_CODE equ 0x99
F11_CODE equ 0x9A
F12_CODE equ 0x9B

; Key press/release codes
KEY_LSHIFT_PRESS equ 0x2A
KEY_LSHIFT_RELEASE equ 0xAA
KEY_RSHIFT_PRESS equ 0x36
KEY_RSHIFT_RELEASE equ 0xB6
KEY_CTRL_PRESS equ 0x1D
KEY_CTRL_RELEASE equ 0x9D
KEY_ALT_PRESS equ 0x38
KEY_ALT_RELEASE equ 0xB8
KEY_CAPS_PRESS equ 0x3A
KEY_NUM_LOCK_PRESS equ 0x45
KEY_SCROLL_LOCK_PRESS equ 0x46

; Data section
SECTION .data

; Keyboard state variables
extended_key db 0
key_released db 0
kbd_flags db 0

; Keyboard layout tables - Perfect PS/2 Set 1 mapping
; Standard PC keyboard scancode mapping (Set 1)
; Scancodes 0x00-0x7F mapped to ASCII characters
kbd_us:
    db 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8      ; 0x01-0x0F
    db 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 10         ; 0x10-0x1C
    db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0         ; 0x1D-0x2A
    db 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0               ; 0x2B-0x36
    db 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                       ; 0x37-0x46
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x47-0x56
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x57-0x66
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x67-0x76
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x77-0x7F

kbd_us_shift:
    db 0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8      ; 0x01-0x0F
    db 9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 10        ; 0x10-0x1C
    db 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0         ; 0x1D-0x2A
    db 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0               ; 0x2B-0x36
    db 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                       ; 0x37-0x46
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x47-0x56
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x57-0x66
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x67-0x76
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x77-0x7F

SECTION .text

; Function: kbd_wait - Wait for keyboard controller ready
kbd_wait:
    push eax
.kbd_wait_loop:
    in al, KBD_STATUS_PORT
    test al, 0x02
    jnz .kbd_wait_loop
    pop eax
    ret

; Function: init_keyboard - Initialize keyboard controller
init_keyboard:
    push eax
    
    call kbd_wait
    mov al, 0xAE
    out KBD_COMMAND_PORT, al
    
    call kbd_wait
    mov al, 0x20
    out KBD_COMMAND_PORT, al
    
    call kbd_wait
    in al, KBD_DATA_PORT
    or al, 1
    push eax
    
    call kbd_wait
    mov al, 0x60
    out KBD_COMMAND_PORT, al
    
    call kbd_wait
    pop eax
    out KBD_DATA_PORT, al
    
    call kbd_wait
    pop eax
    ret

; Function: get_char - Get character from keyboard
get_char:
    push ebx
    push ecx
    push edx
    push esi
    push edi
    
    mov byte [extended_key], 0
    mov byte [key_released], 0
    
.get_char_loop:
    ; Check if data is available
    in al, KBD_STATUS_PORT
    test al, 0x01
    jz .get_char_loop
    
    ; Read scancode
    in al, KBD_DATA_PORT
    mov bl, al
    
    ; Check for extended key prefix
    cmp bl, 0xE0
    je .handle_extended
    
    ; Check if key was released
    test bl, 0x80
    jnz .handle_release
    
    ; Key press
    mov byte [key_released], 0
    jmp .process_key
    
.handle_extended:
    mov byte [extended_key], 1
    jmp .get_char_loop
    
.handle_release:
    mov byte [key_released], 1
    and bl, 0x7F
    
    ; Handle modifier key releases
    cmp bl, (KEY_LSHIFT_PRESS & 0x7F)
    je .release_shift
    cmp bl, (KEY_RSHIFT_PRESS & 0x7F)
    je .release_shift
    cmp bl, (KEY_CTRL_PRESS & 0x7D)
    je .release_ctrl
    cmp bl, (KEY_ALT_PRESS & 0x7F)
    je .release_alt
    
    ; Extended key handling for releases
    cmp byte [extended_key], 1
    jne .get_char_loop
    
    ; Reset extended key flag
    mov byte [extended_key], 0
    jmp .get_char_loop
    
.release_shift:
    and byte [kbd_flags], ~KEY_SHIFT
    jmp .get_char_loop
    
.release_ctrl:
    and byte [kbd_flags], ~KEY_CTRL
    jmp .get_char_loop
    
.release_alt:
    and byte [kbd_flags], ~KEY_ALT
    jmp .get_char_loop
    
.process_key:
    cmp byte [extended_key], 1
    je .process_extended_key
    
    ; Handle modifier key presses
    cmp bl, KEY_LSHIFT_PRESS
    je .press_shift
    cmp bl, KEY_RSHIFT_PRESS
    je .press_shift
    cmp bl, KEY_CTRL_PRESS
    je .press_ctrl
    cmp bl, KEY_ALT_PRESS
    je .press_alt
    cmp bl, KEY_CAPS_PRESS
    je .toggle_caps
    cmp bl, KEY_NUM_LOCK_PRESS
    je .toggle_num_lock
    cmp bl, KEY_SCROLL_LOCK_PRESS
    je .toggle_scroll_lock
    
    ; Handle function keys
    cmp bl, KEY_F1
    jb .check_normal_key
    cmp bl, KEY_F10
    jbe .handle_function_key
    cmp bl, KEY_F11
    je .handle_function_key
    cmp bl, KEY_F12
    je .handle_function_key
    
.check_normal_key:
    cmp bl, 1
    je .return_esc
    cmp bl, 128
    jae .get_char_loop
    
    ; Get character from keyboard table
    movzx esi, bl
    mov al, byte [kbd_flags]
    test al, KEY_SHIFT
    jz .use_normal_table
    
    ; Use shift table
    mov al, byte [kbd_us_shift + esi]
    jmp .process_character
    
.use_normal_table:
    mov al, byte [kbd_us + esi]
    
.process_character:
    cmp al, 0
    je .get_char_loop
    
    ; Handle caps lock
    cmp al, 'a'
    jb .check_caps_upper
    cmp al, 'z'
    jbe .handle_lower_case
    cmp al, 'A'
    jb .return_char
    cmp al, 'Z'
    jbe .handle_upper_case
    jmp .return_char
    
.handle_lower_case:
    mov bl, byte [kbd_flags]
    test bl, KEY_CAPS
    jz .return_char
    sub al, 'a' - 'A'
    jmp .return_char
    
.handle_upper_case:
    mov bl, byte [kbd_flags]
    test bl, KEY_CAPS
    jnz .check_shift_for_upper
    test bl, KEY_SHIFT
    jz .return_char
    add al, 'a' - 'A'
    jmp .return_char
    
.check_shift_for_upper:
    test bl, KEY_SHIFT
    jz .return_char
    add al, 'a' - 'A'
    jmp .return_char
    
.check_caps_upper:
    mov bl, byte [kbd_flags]
    test bl, KEY_CAPS
    jz .return_char
    cmp al, 'A'
    jb .return_char
    cmp al, 'Z'
    ja .return_char
    add al, 'a' - 'A'
    jmp .return_char
    
.press_shift:
    or byte [kbd_flags], KEY_SHIFT
    jmp .get_char_loop
    
.press_ctrl:
    or byte [kbd_flags], KEY_CTRL
    jmp .get_char_loop
    
.press_alt:
    or byte [kbd_flags], KEY_ALT
    jmp .get_char_loop
    
.toggle_caps:
    xor byte [kbd_flags], KEY_CAPS
    jmp .get_char_loop
    
.toggle_num_lock:
    xor byte [kbd_flags], KEY_NUM_LOCK
    jmp .get_char_loop
    
.toggle_scroll_lock:
    xor byte [kbd_flags], KEY_SCROLL_LOCK
    jmp .get_char_loop
    
.handle_function_key:
    mov al, F1_CODE
    add al, bl
    sub al, KEY_F1
    jmp .return_char
    
.process_extended_key:
    mov byte [extended_key], 0
    cmp bl, KEY_ARROW_UP
    je .return_arrow_up
    cmp bl, KEY_ARROW_DOWN
    je .return_arrow_down
    cmp bl, KEY_ARROW_LEFT
    je .return_arrow_left
    cmp bl, KEY_ARROW_RIGHT
    je .return_arrow_right
    cmp bl, KEY_HOME
    je .return_home
    cmp bl, KEY_END
    je .return_end
    cmp bl, KEY_PAGE_UP
    je .return_pgup
    cmp bl, KEY_PAGE_DOWN
    je .return_pgdn
    cmp bl, KEY_INSERT
    je .return_ins
    cmp bl, KEY_DELETE
    je .return_del
    jmp .get_char_loop
    
.return_esc:
    mov eax, KEY_ESC_CODE
    jmp .done
    
.return_arrow_up:
    mov eax, ARROW_UP
    jmp .done
    
.return_arrow_down:
    mov eax, ARROW_DOWN
    jmp .done
    
.return_arrow_left:
    mov eax, ARROW_LEFT
    jmp .done
    
.return_arrow_right:
    mov eax, ARROW_RIGHT
    jmp .done
    
.return_home:
    mov eax, KEY_HOME_CODE
    jmp .done
    
.return_end:
    mov eax, KEY_END_CODE
    jmp .done
    
.return_pgup:
    mov eax, KEY_PGUP_CODE
    jmp .done
    
.return_pgdn:
    mov eax, KEY_PGDN_CODE
    jmp .done
    
.return_ins:
    mov eax, KEY_INS_CODE
    jmp .done
    
.return_del:
    mov eax, KEY_DEL_CODE
    jmp .done
    
.return_char:
    ; Return character in AL, zero extend to EAX
    movzx eax, al
    
.done:
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    ret