#ifndef KEYBOARD_H
#define KEYBOARD_H

// Keyboard ports
#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64
#define KBD_COMMAND_PORT 0x64

// Key flags (Linux style - match keyboard.asm)
#define KEY_LSHIFT 0x01
#define KEY_RSHIFT 0x02
#define KEY_CTRL 0x04
#define KEY_ALT 0x08
#define KEY_CAPS 0x10
#define KEY_NUM 0x20
#define KEY_SCROLL 0x40

// LED flags
#define LED_NUM 0x02
#define LED_CAPS 0x04
#define LED_SCROLL 0x01

// Special key scancodes
#define KEY_ARROW_UP    0x48
#define KEY_ARROW_DOWN  0x50
#define KEY_ARROW_LEFT  0x4B
#define KEY_ARROW_RIGHT 0x4D
#define KEY_HOME        0x47
#define KEY_END         0x4F
#define KEY_PAGE_UP     0x49
#define KEY_PAGE_DOWN   0x51
#define KEY_INSERT      0x52
#define KEY_DELETE      0x53
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_F11         0x57
#define KEY_F12         0x58

// Special key codes
#define ARROW_LEFT      0x80
#define ARROW_RIGHT     0x81
#define ARROW_UP        0x82
#define ARROW_DOWN      0x83
#define KEY_HOME_CODE   0x84
#define KEY_END_CODE    0x85
#define KEY_PGUP_CODE   0x86
#define KEY_PGDN_CODE   0x87
#define KEY_INS_CODE    0x88
#define KEY_DEL_CODE    0x89
#define KEY_ESC_CODE    0x1B
#define F1_CODE         0x90
#define F2_CODE         0x91
#define F3_CODE         0x92
#define F4_CODE         0x93
#define F5_CODE         0x94
#define F6_CODE         0x95
#define F7_CODE         0x96
#define F8_CODE         0x97
#define F9_CODE         0x98
#define F10_CODE        0x99
#define F11_CODE        0x9A
#define F12_CODE        0x9B

// Key press/release codes
#define KEY_LSHIFT_PRESS 0x2A
#define KEY_LSHIFT_RELEASE 0xAA
#define KEY_RSHIFT_PRESS 0x36
#define KEY_RSHIFT_RELEASE 0xB6
#define KEY_CTRL_PRESS 0x1D
#define KEY_CTRL_RELEASE 0x9D
#define KEY_ALT_PRESS 0x38
#define KEY_ALT_RELEASE 0xB8
#define KEY_CAPS_PRESS 0x3A
#define KEY_NUM_LOCK_PRESS 0x45
#define KEY_SCROLL_LOCK_PRESS 0x46

// Global variables
extern unsigned char kbd_flags;
extern unsigned char kbd_us[128];
extern unsigned char kbd_us_shift[128];

// Function declarations
void kbd_wait();
void init_keyboard();
int get_char();

#endif // KEYBOARD_H