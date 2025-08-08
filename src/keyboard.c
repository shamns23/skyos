#include "keyboard.h"
#include "io.h"

// Keyboard state variables
static int extended_key = 0;
static int key_released = 0;
unsigned char kbd_flags = 0;

// Keyboard layout tables
unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, 
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, 
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void kbd_wait() {
    while (inb(KBD_STATUS_PORT) & 0x02);
}

void init_keyboard() {
    kbd_wait();
    outb(KBD_COMMAND_PORT, 0xAE);
    kbd_wait();
    outb(KBD_COMMAND_PORT, 0x20);
    kbd_wait();
    unsigned char status = inb(KBD_DATA_PORT) | 1;
    kbd_wait();
    outb(KBD_COMMAND_PORT, 0x60);
    kbd_wait();
    outb(KBD_DATA_PORT, status);
    kbd_wait();
}

int get_char() {
    unsigned char scancode;
    
    while (1) {
        if (!(inb(KBD_STATUS_PORT) & 0x01)) {
            continue;
        }
        
        scancode = inb(KBD_DATA_PORT);
        
        if (scancode == 0xE0) {
            extended_key = 1;
            continue;
        }
        
        if (scancode & 0x80) {
            key_released = 1;
            scancode &= 0x7F;
        } else {
            key_released = 0;
        }
        
        if (extended_key) {
            extended_key = 0;
            if (!key_released) {
                switch (scancode) {
                    case KEY_ARROW_UP: return ARROW_UP;
                    case KEY_ARROW_DOWN: return ARROW_DOWN;
                    case KEY_ARROW_LEFT: return ARROW_LEFT;
                    case KEY_ARROW_RIGHT: return ARROW_RIGHT;
                    case KEY_HOME: return KEY_HOME_CODE;
                    case KEY_END: return KEY_END_CODE;
                    case KEY_PAGE_UP: return KEY_PGUP_CODE;
                    case KEY_PAGE_DOWN: return KEY_PGDN_CODE;
                    case KEY_INSERT: return KEY_INS_CODE;
                    case KEY_DELETE: return KEY_DEL_CODE;
                }
            }
            continue;
        }
        
        if (key_released) {
            switch (scancode) {
                case KEY_LSHIFT_PRESS & 0x7F:
                case KEY_RSHIFT_PRESS & 0x7F:
                    kbd_flags &= ~KEY_SHIFT;
                    break;
                case KEY_CTRL_PRESS & 0x7F:
                    kbd_flags &= ~KEY_CTRL;
                    break;
                case KEY_ALT_PRESS & 0x7F:
                    kbd_flags &= ~KEY_ALT;
                    break;
            }
            continue;
        }
        
        switch (scancode) {
            case KEY_LSHIFT_PRESS:
            case KEY_RSHIFT_PRESS:
                kbd_flags |= KEY_SHIFT;
                continue;
            case KEY_CTRL_PRESS:
                kbd_flags |= KEY_CTRL;
                continue;
            case KEY_ALT_PRESS:
                kbd_flags |= KEY_ALT;
                continue;
            case KEY_CAPS_PRESS:
                kbd_flags ^= KEY_CAPS;
                continue;
            case KEY_NUM_LOCK_PRESS:
                kbd_flags ^= KEY_NUM_LOCK;
                continue;
            case KEY_SCROLL_LOCK_PRESS:
                kbd_flags ^= KEY_SCROLL_LOCK;
                continue;
        }
        
        if (scancode >= KEY_F1 && scancode <= KEY_F10) {
            return F1_CODE + (scancode - KEY_F1);
        }
        if (scancode == KEY_F11) return F11_CODE;
        if (scancode == KEY_F12) return F12_CODE;
        
        if (scancode == 1) return KEY_ESC_CODE;
        
        if (scancode < 128) {
            char c;
            if (kbd_flags & KEY_SHIFT) {
                c = kbd_us_shift[scancode];
            } else {
                c = kbd_us[scancode];
            }
            
            if (c >= 'a' && c <= 'z' && (kbd_flags & KEY_CAPS)) {
                c = c - 'a' + 'A';
            } else if (c >= 'A' && c <= 'Z' && (kbd_flags & KEY_CAPS) && !(kbd_flags & KEY_SHIFT)) {
                c = c - 'A' + 'a';
            }
            
            return c;
        }
    }
}