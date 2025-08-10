/*
 * Simple keyboard driver for custom OS
 * Compatible with the existing keyboard.h interface
 */

#include "../include/keyboard.h"
#include "../include/io.h"

// Global keyboard state variables
unsigned char kbd_flags = 0;
unsigned char kbd_leds = 0;
unsigned char extended_key = 0;
unsigned char e1_prefix = 0;

// US keyboard layout (normal)
unsigned char kbd_us[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// US keyboard layout (shift)
unsigned char kbd_us_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Numeric keypad layout
unsigned char num_table[13] = "789-456+1230.";

// Cursor keys layout
unsigned char cur_table[13] = "1A5-DGC+4B623";

// Wait for keyboard controller to be ready
void kbd_wait() {
    while (inb(KBD_STATUS_PORT) & 0x02) {
        // Wait until input buffer is empty
    }
}

// Wait for keyboard controller output
void kb_wait() {
    while (!(inb(KBD_STATUS_PORT) & 0x01)) {
        // Wait until output buffer is full
    }
}

// Set keyboard LEDs
void set_leds() {
    kbd_wait();
    outb(KBD_DATA_PORT, 0xED);  // Set LEDs command
    kbd_wait();
    outb(KBD_DATA_PORT, kbd_leds & 0x07);  // LED state
}

// Initialize keyboard
void init_keyboard() {
    // Clear all flags
    kbd_flags = 0;
    kbd_leds = 0;
    extended_key = 0;
    e1_prefix = 0;
    
    // Enable keyboard
    kbd_wait();
    outb(KBD_COMMAND_PORT, 0xAE);  // Enable keyboard interface
    
    // Set keyboard to scan code set 1
    kbd_wait();
    outb(KBD_DATA_PORT, 0xF0);
    kbd_wait();
    outb(KBD_DATA_PORT, 0x01);
    
    // Enable keyboard
    kbd_wait();
    outb(KBD_DATA_PORT, 0xF4);
}

// Handle modifier keys
void handle_modifier(unsigned char scancode) {
    switch (scancode) {
        case KEY_LSHIFT_PRESS:
            kbd_flags |= KEY_LSHIFT;
            break;
        case KEY_LSHIFT_RELEASE:
            kbd_flags &= ~KEY_LSHIFT;
            break;
        case KEY_RSHIFT_PRESS:
            kbd_flags |= KEY_RSHIFT;
            break;
        case KEY_RSHIFT_RELEASE:
            kbd_flags &= ~KEY_RSHIFT;
            break;
        case KEY_CTRL_PRESS:
            kbd_flags |= KEY_CTRL;
            break;
        case KEY_CTRL_RELEASE:
            kbd_flags &= ~KEY_CTRL;
            break;
        case KEY_ALT_PRESS:
            kbd_flags |= KEY_ALT;
            break;
        case KEY_ALT_RELEASE:
            kbd_flags &= ~KEY_ALT;
            break;
        case KEY_CAPS_PRESS:
            kbd_leds ^= LED_CAPS;
            set_leds();
            break;
        case KEY_NUM_LOCK_PRESS:
            kbd_leds ^= LED_NUM;
            set_leds();
            break;
        case KEY_SCROLL_LOCK_PRESS:
            kbd_leds ^= LED_SCROLL;
            set_leds();
            break;
    }
}

// Get character from keyboard
int get_char() {
    unsigned char scancode;
    unsigned char *table;
    unsigned char ch;
    
    // Check if data is available
    if (!(inb(KBD_STATUS_PORT) & 0x01)) {
        return 0;  // No data available
    }
    
    scancode = inb(KBD_DATA_PORT);
    
    // Handle extended key prefixes
    if (scancode == 0xE0) {
        extended_key = 1;
        return 0;
    }
    
    if (scancode == 0xE1) {
        e1_prefix = 1;
        return 0;
    }
    
    // Handle key release (high bit set)
    if (scancode & 0x80) {
        scancode &= 0x7F;  // Remove release bit
        
        // Handle modifier key releases
        handle_modifier(scancode | 0x80);
        
        extended_key = 0;
        return 0;
    }
    
    // Handle modifier key presses
    if (scancode == KEY_LSHIFT_PRESS || scancode == KEY_RSHIFT_PRESS ||
        scancode == KEY_CTRL_PRESS || scancode == KEY_ALT_PRESS ||
        scancode == KEY_CAPS_PRESS || scancode == KEY_NUM_LOCK_PRESS ||
        scancode == KEY_SCROLL_LOCK_PRESS) {
        handle_modifier(scancode);
        extended_key = 0;
        return 0;
    }
    
    // Handle extended keys
    if (extended_key) {
        extended_key = 0;
        switch (scancode) {
            case KEY_ARROW_UP:
                return ARROW_UP;
            case KEY_ARROW_DOWN:
                return ARROW_DOWN;
            case KEY_ARROW_LEFT:
                return ARROW_LEFT;
            case KEY_ARROW_RIGHT:
                return ARROW_RIGHT;
            case KEY_HOME:
                return KEY_HOME_CODE;
            case KEY_END:
                return KEY_END_CODE;
            case KEY_PAGE_UP:
                return KEY_PGUP_CODE;
            case KEY_PAGE_DOWN:
                return KEY_PGDN_CODE;
            case KEY_INSERT:
                return KEY_INS_CODE;
            case KEY_DELETE:
                return KEY_DEL_CODE;
            default:
                return 0;
        }
    }
    
    // Handle function keys
    if (scancode >= KEY_F1 && scancode <= KEY_F10) {
        return F1_CODE + (scancode - KEY_F1);
    }
    if (scancode == KEY_F11) {
        return F11_CODE;
    }
    if (scancode == KEY_F12) {
        return F12_CODE;
    }
    
    // Handle numeric keypad
    if (scancode >= 0x47 && scancode <= 0x53) {
        if (kbd_leds & LED_NUM) {
            // Num lock on - return numbers
            return num_table[scancode - 0x47];
        } else {
            // Num lock off - return cursor keys
            switch (scancode) {
                case 0x47: return KEY_HOME_CODE;
                case 0x48: return ARROW_UP;
                case 0x49: return KEY_PGUP_CODE;
                case 0x4B: return ARROW_LEFT;
                case 0x4D: return ARROW_RIGHT;
                case 0x4F: return KEY_END_CODE;
                case 0x50: return ARROW_DOWN;
                case 0x51: return KEY_PGDN_CODE;
                case 0x52: return KEY_INS_CODE;
                case 0x53: return KEY_DEL_CODE;
                default: return 0;
            }
        }
    }
    
    // Handle normal keys
    if (scancode >= 128) {
        return 0;
    }
    
    // Choose the appropriate table
    if (kbd_flags & (KEY_LSHIFT | KEY_RSHIFT)) {
        table = kbd_us_shift;
    } else {
        table = kbd_us;
    }
    
    ch = table[scancode];
    
    // Handle caps lock for letters
    if (ch >= 'a' && ch <= 'z') {
        if (kbd_leds & LED_CAPS) {
            ch = ch - 'a' + 'A';
        }
    } else if (ch >= 'A' && ch <= 'Z') {
        if (kbd_leds & LED_CAPS) {
            ch = ch - 'A' + 'a';
        }
    }
    
    // Handle control key combinations
    if (kbd_flags & KEY_CTRL) {
        if (ch >= 'a' && ch <= 'z') {
            ch = ch - 'a' + 1;  // Ctrl+A = 1, Ctrl+B = 2, etc.
        } else if (ch >= 'A' && ch <= 'Z') {
            ch = ch - 'A' + 1;
        }
    }
    
    return ch;
}
