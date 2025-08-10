/*
                            ________    _________ 
   ____  __________________ \_____  \  /   _____/ 
  /  _ \/  ___/\___   /  _ \ /   |   \ \_____  \  
 (  <_> )___ \  /    (  <_> )    |    \/        \ 
  \____/____  >/_____ \____/\_______  /_______  / 
            \/       \/             \/        \/
 * Keyboard driver for oszoOS
 * Converted from Linux keyboard.S to C implementation
 * Compatible with oszoOS system architecture
 */

#include "keyboard.h"
#include "io.h"


// Global keyboard state variables
unsigned char kbd_flags = 0;
unsigned char kbd_leds = 2; // num-lock on by default
unsigned char extended_key = 0;
unsigned char e1_prefix = 0;

// US keyboard layout (normal)
unsigned char kbd_us[128] = {
    // 0x00-0x0F: Special keys and numbers
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    // 0x10-0x1F: QWERTY row
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    // 0x20-0x2F: ASDF row
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    // 0x30-0x3F: ZXCV row, modifiers, and space
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    // 0x40-0x4F: F-keys and numeric keypad
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    // 0x50-0x5F: Numeric keypad and F-keys
    '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // 0x60-0x7F: Unused
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Numeric keypad layout
unsigned char num_table[13] = "789 456 1230,";

// Cursor keys layout
unsigned char cur_table[13] = "HA5 DGC YB623";

// US keyboard layout (shift)
unsigned char kbd_us_shift[128] = {
    // 0x00-0x0F: Special keys and symbols
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    // 0x10-0x1F: QWERTY row (shifted)
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    // 0x20-0x2F: ASDF row (shifted)
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    // 0x30-0x3F: ZXCV row (shifted), modifiers, and space
    'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    // 0x40-0x4F: F-keys and numeric keypad (same as non-shifted)
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    // 0x50-0x5F: Numeric keypad and F-keys (same as non-shifted)
    '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // 0x60-0x7F: Unused
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Wait for keyboard controller to be ready
void kbd_wait(void) {
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
}

// Alternative wait function (compatibility)
void kb_wait(void) {
    kbd_wait();
}

// Set keyboard LEDs
void set_leds(void) {
    kbd_wait();
    outb(KEYBOARD_DATA_PORT, 0xED); // Set LEDs command
    kbd_wait();
    outb(KEYBOARD_DATA_PORT, kbd_leds);
}

// Handle modifier keys
void handle_modifier_key(unsigned char scancode, unsigned char pressed) {
    switch (scancode) {
        case KEY_LSHIFT_SC:
            if (pressed) kbd_flags |= KEY_LSHIFT;
            else kbd_flags &= ~KEY_LSHIFT;
            break;
        case KEY_RSHIFT_SC:
            if (pressed) kbd_flags |= KEY_RSHIFT;
            else kbd_flags &= ~KEY_RSHIFT;
            break;
        case KEY_LCTRL:
            if (pressed) {
                if (extended_key) kbd_flags |= (KEY_CTRL << 1);
                else kbd_flags |= KEY_CTRL;
            } else {
                if (extended_key) kbd_flags &= ~(KEY_CTRL << 1);
                else kbd_flags &= ~KEY_CTRL;
            }
            break;
        case KEY_LALT:
            if (pressed) {
                if (extended_key) kbd_flags |= (KEY_ALT << 1);
                else kbd_flags |= KEY_ALT;
            } else {
                if (extended_key) kbd_flags &= ~(KEY_ALT << 1);
                else kbd_flags &= ~KEY_ALT;
            }
            break;
        case KEY_CAPS_SC:
            if (pressed) {
                if (!(kbd_flags & KEY_E0)) {
                    kbd_leds ^= LED_CAPS;
                    kbd_flags ^= KEY_CAPS;
                    kbd_flags |= KEY_E0;
                    set_leds();
                }
            } else {
                kbd_flags &= ~KEY_E0;
            }
            break;
        case KEY_NUM_SC:
            if (pressed) {
                kbd_leds ^= LED_NUM;
                set_leds();
            }
            break;
        case KEY_SCROLL_SC:
            if (pressed) {
                kbd_leds ^= LED_SCROLL;
                set_leds();
            }
            break;
    }
}

// Handle cursor/numeric keypad
int handle_cursor_keys(unsigned char scancode) {
    unsigned char key_index = scancode - 0x47;
    
    if (key_index > 12) return 0;
    
    // Check for Ctrl+Alt+Del
    if (scancode == KEY_DELETE && (kbd_flags & KEY_CTRL) && (kbd_flags & KEY_ALT)) {
        // Reboot system (placeholder)
        return 0;
    }
    
    // Return special key codes for arrow keys
    switch (scancode) {
        case 0x48: return extended_key ? ARROW_UP : '8';
        case 0x50: return extended_key ? ARROW_DOWN : '2';
        case 0x4B: return extended_key ? ARROW_LEFT : '4';
        case 0x4D: return extended_key ? ARROW_RIGHT : '6';
        case 0x47: return extended_key ? KEY_HOME_CODE : '7';
        case 0x4F: return extended_key ? KEY_END_CODE : '1';
        case 0x53: return extended_key ? KEY_DEL_CODE : '.';
    }
    
    // E0 prefix forces cursor movement
    if (extended_key) {
        return cur_table[key_index];
    }
    
    // Num lock off or shift pressed forces cursor
    if (!(kbd_leds & LED_NUM) || (kbd_flags & (KEY_LSHIFT | KEY_RSHIFT))) {
        return cur_table[key_index];
    }
    
    // Return numeric keypad character
    return num_table[key_index];
}

// Handle function keys
char handle_function_keys(unsigned char scancode) {
    if (scancode >= KEY_F1 && scancode <= KEY_F10) {
        // Function keys - return special codes
        return 0xF0 + (scancode - KEY_F1);
    }
    return 0;
}

// Initialize keyboard
void init_keyboard(void) {
    kbd_flags = 0;
    extended_key = 0;
    e1_prefix = 0;
    kbd_leds = 2; // Num lock on
    set_leds();
}

// Get character from keyboard
int get_char(void) {
    unsigned char scancode;
    unsigned char pressed;
    char ch;
    
    while (1) {
        // Wait for key press
        while (!(inb(KEYBOARD_STATUS_PORT) & 0x01));
        
        scancode = inb(KEYBOARD_DATA_PORT);
        
        // Handle extended key prefixes
        if (scancode == 0xE0) {
            extended_key = 1;
            continue;
        }
        
        if (scancode == 0xE1) {
            e1_prefix = 1;
            continue;
        }
        
        // Determine if key is pressed or released
        pressed = !(scancode & KEY_RELEASE);
        scancode &= 0x7F; // Remove release bit
        
        // Handle modifier keys
        if (scancode == KEY_LSHIFT_SC || scancode == KEY_RSHIFT_SC ||
            scancode == KEY_LCTRL || scancode == KEY_LALT ||
            scancode == KEY_CAPS_SC || scancode == KEY_NUM_SC ||
            scancode == KEY_SCROLL_SC) {
            handle_modifier_key(scancode, pressed);
            extended_key = 0;
            continue;
        }
        
        // Only process key press events for regular keys
        if (!pressed) {
            extended_key = 0;
            continue;
        }
        
        // Handle cursor/numeric keypad
        if (scancode >= 0x47 && scancode <= 0x53) {
            ch = handle_cursor_keys(scancode);
            if (ch) {
                extended_key = 0;
                return ch;
            }
        }
        
        // Handle function keys
        ch = handle_function_keys(scancode);
        if (ch) {
            extended_key = 0;
            return ch;
        }
        
        // Handle ESC key
        if (scancode == KEY_ESC) {
            extended_key = 0;
            return KEY_ESC_CODE;
        }
        
        // Handle regular keys
        if (scancode < 128) {
            if (kbd_flags & (KEY_LSHIFT | KEY_RSHIFT)) {
                ch = kbd_us_shift[scancode];
            } else {
                ch = kbd_us[scancode];
            }
            
            // Handle caps lock for letters
            if (ch >= 'a' && ch <= 'z' && (kbd_leds & LED_CAPS)) {
                ch = ch - 'a' + 'A';
            } else if (ch >= 'A' && ch <= 'Z' && (kbd_leds & LED_CAPS) && 
                      !(kbd_flags & (KEY_LSHIFT | KEY_RSHIFT))) {
                ch = ch - 'A' + 'a';
            }
            
            if (ch) {
                extended_key = 0;
                return ch;
            }
        }
        
        extended_key = 0;
    }
}

// Keyboard interrupt handler (placeholder)
void keyboard_interrupt(void) {
    // This would be called by the interrupt handler
    // For now, we use polling in get_char()
}
