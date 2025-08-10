; oszoOS Keyboard Driver - Enhanced Linux 0.11 Style Implementation
; Based on Linux 0.11 keyboard.S with modern adaptations

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

; Key flags (Linux style)
KEY_LSHIFT equ 0x01
KEY_RSHIFT equ 0x02
KEY_CTRL equ 0x04
KEY_ALT equ 0x08
KEY_CAPS equ 0x10
KEY_NUM equ 0x20
KEY_SCROLL equ 0x40

; LED flags
LED_NUM equ 0x02
LED_CAPS equ 0x04
LED_SCROLL equ 0x01

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

; Keyboard state variables (Linux style)
extended_key db 0        ; Extended key flag (0xE0 prefix)
e1_prefix db 0         ; E1 prefix flag
kbd_flags db 0         ; Current modifier state
kbd_leds db 2          ; LED state (num-lock on by default)

; Enhanced keyboard layout tables - Linux 0.11 style
; US keyboard layout with proper scan code mapping
kbd_us:
    db 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8      ; 0x00-0x0E
    db 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 10         ; 0x0F-0x1C
    db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0         ; 0x1D-0x2A
    db 0, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0               ; 0x2B-0x38
    db ' ', '*', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                       ; 0x39-0x48
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x49-0x58
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x59-0x68
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x69-0x78
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x79-0x7F

kbd_us_shift:
    db 0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8      ; 0x00-0x0E
    db 9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 10        ; 0x0F-0x1C
    db 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0         ; 0x1D-0x2A
    db 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0               ; 0x2B-0x38
    db ' ', '*', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                       ; 0x39-0x48
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x49-0x58
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x59-0x68
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x69-0x78
    db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ; 0x79-0x7F

; Numeric keypad table (when num-lock is on)
num_table:
    db '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'          ; 0x47-0x53

; Cursor key table (when num-lock is off or shift is pressed)
cur_table:
    db 'H', '8', '9', 'A', '4', '5', '6', 'B', '1', '2', '3', 'C', 'D'          ; 0x47-0x53

SECTION .text

; Function: kbd_wait - Wait for keyboard controller ready (Linux style)
kbd_wait:
    push eax
.kbd_wait_loop:
    in al, KBD_STATUS_PORT
    test al, 0x02
    jnz .kbd_wait_loop
    pop eax
    ret

; Function: kb_wait - Wait for keyboard ACK (Linux style)
kb_wait:
    push eax
.kb_wait_loop:
    in al, KBD_STATUS_PORT
    test al, 0x01
    jz .kb_wait_loop
    in al, KBD_DATA_PORT
    cmp al, 0xFA
    jne .kb_wait_loop
    pop eax
    ret

; Function: set_leds - Set keyboard LEDs (Linux style)
set_leds:
    push eax
    push ebx
    
    call kbd_wait
    mov al, 0xED        ; Set LEDs command
    out KBD_DATA_PORT, al
    
    call kb_wait
    mov al, [kbd_leds]
    out KBD_DATA_PORT, al
    
    pop ebx
    pop eax
    ret

; Function: init_keyboard - Initialize keyboard controller (Linux style)
init_keyboard:
    push eax
    
    ; Disable keyboard and mouse during initialization
    call kbd_wait
    mov al, 0xAD        ; Disable keyboard
    out KBD_COMMAND_PORT, al
    call kbd_wait
    mov al, 0xA7        ; Disable mouse (if present)
    out KBD_COMMAND_PORT, al
    
    ; Flush the output buffer
.flush_loop:
    call kbd_wait
    in al, KBD_STATUS_PORT
    test al, 0x01
    jz .flush_done
    in al, KBD_DATA_PORT
    jmp .flush_loop
.flush_done:
    
    ; Set controller configuration byte
    call kbd_wait
    mov al, 0x20        ; Read configuration byte
    out KBD_COMMAND_PORT, al
    
    call kbd_wait
    in al, KBD_DATA_PORT
    and al, 0x7F        ; Disable keyboard interrupts (using polling)
    push eax
    
    ; Write configuration byte back
    call kbd_wait
    mov al, 0x60        ; Write configuration byte
    out KBD_COMMAND_PORT, al
    
    call kbd_wait
    pop eax
    out KBD_DATA_PORT, al
    
    ; Perform keyboard self-test
    call kbd_wait
    mov al, 0xAA        ; Self-test command
    out KBD_COMMAND_PORT, al
    call kb_wait
    
    ; Enable keyboard
    call kbd_wait
    mov al, 0xAE        ; Enable keyboard
    out KBD_COMMAND_PORT, al
    
    ; Reset keyboard
    call kbd_wait
    mov al, 0xFF        ; Reset keyboard
    out KBD_DATA_PORT, al
    call kb_wait
    
    ; Set LEDs to default (num-lock on)
    mov byte [kbd_leds], LED_NUM
    call set_leds
    
    ; Clear keyboard flags
    mov byte [kbd_flags], 0
    mov byte [extended_key], 0
    mov byte [e1_prefix], 0
    
    call kbd_wait
    pop eax
    ret

; Function: handle_modifier - Handle modifier keys
handle_modifier:
    push eax
    cmp al, KEY_LSHIFT_PRESS
    je .set_lshift
    cmp al, KEY_LSHIFT_RELEASE
    je .unset_lshift
    cmp al, KEY_RSHIFT_PRESS
    je .set_rshift
    cmp al, KEY_RSHIFT_RELEASE
    je .unset_rshift
    cmp al, KEY_CTRL_PRESS
    je .set_ctrl
    cmp al, KEY_CTRL_RELEASE
    je .unset_ctrl
    cmp al, KEY_ALT_PRESS
    je .set_alt
    cmp al, KEY_ALT_RELEASE
    je .unset_alt
    cmp al, KEY_CAPS_PRESS
    je .toggle_caps
    cmp al, KEY_NUM_LOCK_PRESS
    je .toggle_num
    cmp al, KEY_SCROLL_LOCK_PRESS
    je .toggle_scroll
    jmp .done

.set_lshift:
    or byte [kbd_flags], KEY_LSHIFT
    jmp .done
.unset_lshift:
    and byte [kbd_flags], ~KEY_LSHIFT
    jmp .done
.set_rshift:
    or byte [kbd_flags], KEY_RSHIFT
    jmp .done
.unset_rshift:
    and byte [kbd_flags], ~KEY_RSHIFT
    jmp .done
.set_ctrl:
    or byte [kbd_flags], KEY_CTRL
    jmp .done
.unset_ctrl:
    and byte [kbd_flags], ~KEY_CTRL
    jmp .done
.set_alt:
    or byte [kbd_flags], KEY_ALT
    jmp .done
.unset_alt:
    and byte [kbd_flags], ~KEY_ALT
    jmp .done
.toggle_caps:
    xor byte [kbd_leds], LED_CAPS
    call set_leds
    jmp .done
.toggle_num:
    xor byte [kbd_leds], LED_NUM
    call set_leds
    jmp .done
.toggle_scroll:
    xor byte [kbd_leds], LED_SCROLL
    call set_leds
    jmp .done
.done:
    pop eax
    ret

; Function: get_char - Enhanced keyboard input (Linux style)
get_char:
    push ebx
    push ecx
    push edx
    push esi
    push edi
    
    mov byte [extended_key], 0
    mov byte [e1_prefix], 0
    
.get_char_loop:
    ; Check if data is available
    in al, KBD_STATUS_PORT
    test al, 0x01
    jz .get_char_loop
    
    ; Read scancode
    in al, KBD_DATA_PORT
    mov bl, al
    
    ; Handle extended key prefixes
    cmp bl, 0xE0
    je .handle_e0
    cmp bl, 0xE1
    je .handle_e1
    
    ; Handle key release
    test bl, 0x80
    jnz .handle_release
    
    ; Handle key press
    jmp .handle_press
    
.handle_e0:
    mov byte [extended_key], 1
    jmp .get_char_loop
    
.handle_e1:
    mov byte [e1_prefix], 1
    jmp .get_char_loop
    
.handle_release:
    and bl, 0x7F
    call handle_modifier
    jmp .get_char_loop
    
.handle_press:
    ; Handle extended keys
    cmp byte [extended_key], 1
    je .handle_extended_key
    
    ; Handle modifier keys
    cmp bl, KEY_LSHIFT_PRESS
    je .handle_modifier_key
    cmp bl, KEY_RSHIFT_PRESS
    je .handle_modifier_key
    cmp bl, KEY_CTRL_PRESS
    je .handle_modifier_key
    cmp bl, KEY_ALT_PRESS
    je .handle_modifier_key
    cmp bl, KEY_CAPS_PRESS
    je .handle_modifier_key
    cmp bl, KEY_NUM_LOCK_PRESS
    je .handle_modifier_key
    cmp bl, KEY_SCROLL_LOCK_PRESS
    je .handle_modifier_key
    
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
    
    ; Get character from appropriate table
    movzx esi, bl
    mov al, [kbd_flags]
    
    ; Check for numeric keypad
    cmp bl, 0x47
    jb .not_numpad
    cmp bl, 0x53
    ja .not_numpad
    
    ; Handle numeric keypad
    test al, KEY_NUM
    jz .use_cursor_table
    test al, KEY_LSHIFT | KEY_RSHIFT
    jne .use_cursor_table
    mov al, [num_table + esi - 0x47]
    jmp .process_character
    
.use_cursor_table:
    mov al, [cur_table + esi - 0x47]
    jmp .process_character
    
.not_numpad:
    ; Use regular keyboard table
    test al, KEY_LSHIFT | KEY_RSHIFT
    jz .use_normal_table
    mov al, [kbd_us_shift + esi]
    jmp .process_character
    
.use_normal_table:
    mov al, [kbd_us + esi]
    
.process_character:
    cmp al, 0
    je .get_char_loop
    
    ; Handle caps lock for letters
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
    mov bl, [kbd_flags]
    test bl, KEY_CAPS
    jz .return_char
    sub al, 32          ; Convert to uppercase
    jmp .return_char
    
.handle_upper_case:
    mov bl, [kbd_flags]
    test bl, KEY_CAPS
    jz .return_char
    add al, 32          ; Convert to lowercase
    jmp .return_char
    
.check_caps_upper:
    mov bl, [kbd_flags]
    test bl, KEY_CAPS
    jz .return_char
    cmp al, 'A'
    jb .return_char
    cmp al, 'Z'
    ja .return_char
    add al, 32          ; Convert to lowercase
    jmp .return_char
    
.handle_modifier_key:
    call handle_modifier
    jmp .get_char_loop
    
.handle_function_key:
    mov al, F1_CODE
    add al, bl
    sub al, KEY_F1
    jmp .return_char
    
.handle_extended_key:
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
    movzx eax, al
    
.done:
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    ret