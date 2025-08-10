#ifndef KEYBOARD_H
#define KEYBOARD_H

// Keyboard ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Key flags
#define KEY_LSHIFT 0x01
#define KEY_RSHIFT 0x02
#define KEY_CTRL 0x04
#define KEY_ALT 0x08
#define KEY_CAPS 0x10
#define KEY_NUM 0x20
#define KEY_SCROLL 0x40
#define KEY_E0 0x80

// LED flags
#define LED_SCROLL 0x01
#define LED_NUM 0x02
#define LED_CAPS 0x04

// Special key scancodes
#define KEY_ESC 0x01
#define KEY_BACKSPACE 0x0E
#define KEY_TAB 0x0F
#define KEY_ENTER 0x1C
#define KEY_LCTRL 0x1D
#define KEY_LSHIFT_SC 0x2A
#define KEY_RSHIFT_SC 0x36
#define KEY_LALT 0x38
#define KEY_SPACE 0x39
#define KEY_CAPS_SC 0x3A
#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E
#define KEY_F5 0x3F
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_NUM_SC 0x45
#define KEY_SCROLL_SC 0x46

// Special key codes
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D
#define KEY_HOME 0x47
#define KEY_END 0x4F
#define KEY_PAGE_UP 0x49
#define KEY_PAGE_DOWN 0x51
#define KEY_INSERT 0x52
#define KEY_DELETE 0x53

// Key press/release
#define KEY_RELEASE 0x80

// Function key codes
#define F1_CODE 0xF0
#define F2_CODE 0xF1
#define F3_CODE 0xF2
#define F4_CODE 0xF3
#define F5_CODE 0xF4
#define F6_CODE 0xF5
#define F7_CODE 0xF6
#define F8_CODE 0xF7
#define F9_CODE 0xF8
#define F10_CODE 0xF9
#define F11_CODE 0xFA
#define F12_CODE 0xFB

// Arrow keys
#define ARROW_UP 0x148
#define ARROW_DOWN 0x150
#define ARROW_LEFT 0x14B
#define ARROW_RIGHT 0x14D

// Additional key codes
#define KEY_DEL_CODE 0x153
#define KEY_HOME_CODE 0x147
#define KEY_END_CODE 0x14F
#define KEY_ESC_CODE 0x101

// Global variables
extern unsigned char kbd_flags;
extern unsigned char kbd_leds;
extern unsigned char extended_key;
extern unsigned char e1_prefix;

// US keyboard layout
extern unsigned char kbd_us[128];
extern unsigned char kbd_us_shift[128];

// Function declarations
void kbd_wait(void);
void kb_wait(void);
void set_leds(void);
void init_keyboard(void);
int get_char(void);
void keyboard_interrupt(void);

#endif // KEYBOARD_H