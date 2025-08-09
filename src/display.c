#include "display.h"
#include "io.h"

// Global variables
int cursor_x = 0;
int cursor_y = 0;
int current_fg_color = WHITE;
int current_bg_color = BLACK;

void enable_cursor(unsigned char cursor_start, unsigned char cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void update_cursor(int x, int y) {
    unsigned short pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void clear_screen() {
    VideoChar* video = VIDEO_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video[i].character = ' ';
        video[i].attribute = (BLACK << 4) | WHITE;
    }
    cursor_x = 0;
    cursor_y = 0;
    enable_cursor(14, 15);  // Enable cursor with standard shape
    update_cursor(cursor_x, cursor_y);
}

void set_color(int fg, int bg) {
    current_fg_color = fg;
    current_bg_color = bg;
}

void shell_print_colored(const char* str, int fg, int bg) {
    int old_fg = current_fg_color;
    int old_bg = current_bg_color;
    set_color(fg, bg);
    shell_print_string(str);
    set_color(old_fg, old_bg);
}

void scroll_screen() {
    VideoChar* video = VIDEO_MEMORY;
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        video[i] = video[i + VGA_WIDTH];
    }
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        video[i].character = ' ';
        video[i].attribute = (current_bg_color << 4) | current_fg_color;
    }
    cursor_y = VGA_HEIGHT - 1;
}

void shell_print_char(char c) {
    VideoChar* video = VIDEO_MEMORY;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            video[cursor_y * VGA_WIDTH + cursor_x].character = ' ';
            video[cursor_y * VGA_WIDTH + cursor_x].attribute = (current_bg_color << 4) | current_fg_color;
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    } else {
        video[cursor_y * VGA_WIDTH + cursor_x].character = c;
        video[cursor_y * VGA_WIDTH + cursor_x].attribute = (current_bg_color << 4) | current_fg_color;
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
    }
    
    update_cursor(cursor_x, cursor_y);
}

void shell_print_string(const char* str) {
    while (*str) {
        shell_print_char(*str++);
    }
}