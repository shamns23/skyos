#ifndef DISPLAY_H
#define DISPLAY_H

// VGA constants
#define VIDEO_MEMORY (VideoChar*)0xb8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Color definitions
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHT_GRAY 7
#define DARK_GRAY 8
#define LIGHT_BLUE 9
#define LIGHT_GREEN 10
#define LIGHT_CYAN 11
#define LIGHT_RED 12
#define LIGHT_MAGENTA 13
#define YELLOW 14
#define WHITE 15

// Color themes
#define COLOR_ERROR LIGHT_RED
#define COLOR_SUCCESS LIGHT_GREEN
#define COLOR_WARNING YELLOW
#define COLOR_INFO LIGHT_CYAN
#define COLOR_PROMPT LIGHT_CYAN
#define COLOR_DIR LIGHT_BLUE
#define COLOR_FILE WHITE
#define COLOR_EXECUTABLE LIGHT_GREEN
#define COLOR_SPECIAL LIGHT_MAGENTA

// Video character structure
typedef struct {
    char character;
    unsigned char attribute;
} VideoChar;

// Global variables
extern int cursor_x;
extern int cursor_y;
extern int current_fg_color;
extern int current_bg_color;

// Function declarations
void update_cursor(int x, int y);
void clear_screen();
void set_color(int fg, int bg);
void shell_print_colored(const char* str, int fg, int bg);
void scroll_screen();
void shell_print_char(char c);
void shell_print_string(const char* str);

#endif // DISPLAY_H