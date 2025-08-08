#include <stddef.h>
#include "fat32.h"

// Custom string functions to avoid conflicts with builtin functions
#define strlen my_strlen
#define strncmp my_strncmp
#define strncpy my_strncpy
#define strspn my_strspn
#define strcspn my_strcspn 
size_t my_strlen(const char* s) {
    size_t i = 0;
    while (s[i]) i++;
    return i;
}
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}
char* my_strncpy(char* dest, const char* src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}
int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}
int my_strncmp(const char* a, const char* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i] || !a[i] || !b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
    }
    return 0;
}
char* strcat(char* dest, const char* src) {
    char* d = dest + my_strlen(dest);
    while ((*d++ = *src++));
    return dest;
}
char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}
size_t my_strspn(const char* s, const char* accept) {
    size_t count = 0;
    while (*s) {
        const char* a = accept;
        int found = 0;
        while (*a) {
            if (*s == *a) { found = 1; break; }
            a++;
        }
        if (!found) break;
        count++;
        s++;
    }
    return count;
}
size_t my_strcspn(const char* s, const char* reject) {
    int count = 0;
    while (*s) {
        const char* r = reject;
        int found = 0;
        while (*r) {
            if (*s == *r) { found = 1; break; }
            r++;
        }
        if (found) break;
        count++;
        s++;
    }
    return count;
}
char* strtok_r(char* str, const char* delim, char** saveptr) {
    char* token;
    if (str == NULL) str = *saveptr;
    str += my_strspn(str, delim);
    if (*str == '\0') {
        *saveptr = str;
        return NULL;
    }
    token = str;
    str = str + my_strcspn(str, delim);
    if (*str != '\0') {
        *str = '\0';
        str++;
    }
    *saveptr = str;
    return token;
}
#define VIDEO_MEMORY (VideoChar*)0xb8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
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

#define COLOR_ERROR LIGHT_RED
#define COLOR_SUCCESS LIGHT_GREEN
#define COLOR_WARNING YELLOW
#define COLOR_INFO LIGHT_CYAN
#define COLOR_PROMPT LIGHT_CYAN
#define COLOR_DIR LIGHT_BLUE
#define COLOR_FILE WHITE
#define COLOR_EXECUTABLE LIGHT_GREEN
#define COLOR_SPECIAL LIGHT_MAGENTA

#define MAX_FILES 20
#define MAX_FILENAME 32
#define MAX_CONTENT 512
#define MAX_DIRS 10
#define MAX_DIRNAME 32
#define TYPE_FILE 1
#define TYPE_DIR 2
#define EDITOR_MODE_NORMAL 0
#define EDITOR_MODE_INSERT 1
typedef struct {
    char character;
    unsigned char attribute;
} VideoChar;
int current_fg_color = WHITE;
int current_bg_color = BLACK;
char editor_buffer[MAX_CONTENT];
int editor_cursor = 0;
int editor_mode = EDITOR_MODE_NORMAL;
int editor_file_index = -1;
int cursor_x = 0;
int cursor_y = 0;
#define PIT_FREQ 100
volatile unsigned int system_ticks = 0;
extern void irq0_handler();
void shell_print_string(const char* str);
void print_with_pagination(const char* text);
void show_quick_help();
void show_full_help();
void print_tree(int dir_index, int depth);
void show_command_help(const char* command);
void show_ls_help();
void show_cd_help();
void show_mkdir_help();
void show_touch_help();
void show_cat_help();
void show_vim_help();
void show_fat32_help();
void show_color_help();
void show_debug_help();
void show_tree_help();
void show_write_help();
void show_rm_help();
void show_chmod_help();
void show_pwd_help();
void show_sysinfo_help();
void show_fastfetch_help();
void show_shutdown_help();
void show_clear_help();
void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}
void outw(unsigned short port, unsigned short data) {
    asm volatile ("outw %0, %1" : : "a"(data), "Nd"(port));
}
unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
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
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            int src_index = y * VGA_WIDTH + x;
            int dest_index = (y - 1) * VGA_WIDTH + x;
            video[dest_index] = video[src_index];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        video[index].character = ' ';
        video[index].attribute = (current_bg_color << 4) | current_fg_color;
    }
    cursor_y--;
}
void shell_print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == 0x08) { 
        if (cursor_x > 0) {
            cursor_x--;
            VideoChar* video = VIDEO_MEMORY;
            int offset = cursor_y * VGA_WIDTH + cursor_x;
            video[offset].character = ' ';
            video[offset].attribute = (current_bg_color << 4) | current_fg_color;
        }
    } else {
        VideoChar* video = VIDEO_MEMORY;
        int offset = cursor_y * VGA_WIDTH + cursor_x;
        video[offset].character = c;
        video[offset].attribute = (current_bg_color << 4) | current_fg_color;
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen(); 
    }
    update_cursor(cursor_x, cursor_y);
}
void shell_print_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        shell_print_char(str[i]);
    }
}
#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64
#define KBD_COMMAND_PORT 0x64
#define KEY_SHIFT 0x01
#define KEY_CTRL 0x02
#define KEY_ALT 0x04
#define KEY_CAPS 0x08
#define KEY_NUM_LOCK 0x10
#define KEY_SCROLL_LOCK 0x20
static int extended_key = 0;
static int key_released = 0;
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
unsigned char kbd_flags = 0;
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
void kbd_wait() {
    while (inb(KBD_STATUS_PORT) & 0x02);
}
void init_keyboard() {
    kbd_wait();
    outb(KBD_COMMAND_PORT, 0xAE); 
    kbd_wait();
    outb(KBD_COMMAND_PORT, 0x20); 
    kbd_wait();
    unsigned char status = inb(KBD_DATA_PORT);
    status |= 0x01; 
    kbd_wait();
    outb(KBD_COMMAND_PORT, 0x60); 
    kbd_wait();
    outb(KBD_DATA_PORT, status);
}
int get_char() {
    static unsigned char last_scancode = 0;
    static unsigned int repeat_delay = 0;
    unsigned char scancode;
    unsigned char status = inb(KBD_STATUS_PORT);
    if (!(status & 0x01)) {
        return 0;
    }
    scancode = inb(KBD_DATA_PORT);
    if (scancode == last_scancode) {
        repeat_delay++;
        if (repeat_delay < 10000) { 
            return 0;
        }
        repeat_delay = 0;
    } else {
        repeat_delay = 0;
        last_scancode = scancode;
    }
    if (scancode == 0xE0) {
        extended_key = 1;
        return 0;
    }
    if (scancode & 0x80) {
        key_released = 1;
        scancode &= 0x7F;
        if (extended_key) {
            extended_key = 0;
            key_released = 0;
            return 0;
        }
        if (scancode == KEY_LSHIFT_PRESS || scancode == KEY_RSHIFT_PRESS) {
            kbd_flags &= ~KEY_SHIFT;
        } else if (scancode == KEY_CTRL_PRESS) {
            kbd_flags &= ~KEY_CTRL;
        } else if (scancode == KEY_ALT_PRESS) {
            kbd_flags &= ~KEY_ALT;
        }
        key_released = 0;
        return 0;
    }
    if (extended_key) {
        extended_key = 0;
        switch (scancode) {
            case KEY_ARROW_LEFT: 
                return ARROW_LEFT;
            case KEY_ARROW_RIGHT: 
                return ARROW_RIGHT;
            case KEY_ARROW_UP: 
                return ARROW_UP;
            case KEY_ARROW_DOWN: 
                return ARROW_DOWN;
            case KEY_HOME: return KEY_HOME_CODE;
            case KEY_END: return KEY_END_CODE;
            case KEY_PAGE_UP: return KEY_PGUP_CODE;
            case KEY_PAGE_DOWN: return KEY_PGDN_CODE;
            case KEY_INSERT: return KEY_INS_CODE;
            case KEY_DELETE: return KEY_DEL_CODE;
            case KEY_CTRL_PRESS:
                kbd_flags |= KEY_CTRL;
                return 0;
            default: return 0;
        }
    }
    if (scancode >= KEY_F1 && scancode <= KEY_F10) {
        return F1_CODE + (scancode - KEY_F1);
    } else if (scancode == KEY_F11) {
        return F11_CODE;
    } else if (scancode == KEY_F12) {
        return F12_CODE;
    }
    if ((kbd_flags & KEY_CTRL) && scancode < 128) {
        char c = kbd_us[scancode];
        if (c >= 'a' && c <= 'z') {
            return c - 'a' + 1; 
        } else if (c >= 'A' && c <= 'Z') {
            return c - 'A' + 1;
        }
        switch (c) {
            case 'c': case 'C': return 3;   
            case 'd': case 'D': return 4;   
            case 'l': case 'L': return 12;  
            case 'z': case 'Z': return 26;  
        }
    }
    if (scancode == KEY_LSHIFT_PRESS || scancode == KEY_RSHIFT_PRESS) {
        kbd_flags |= KEY_SHIFT;
        return 0;
    } else if (scancode == KEY_CTRL_PRESS) {
        kbd_flags |= KEY_CTRL;
        return 0;
    } else if (scancode == KEY_ALT_PRESS) {
        kbd_flags |= KEY_ALT;
        return 0;
    } else if (scancode == KEY_CAPS_PRESS) {
        kbd_flags ^= KEY_CAPS; 
        return 0;
    } else if (scancode == KEY_NUM_LOCK_PRESS) {
        kbd_flags ^= KEY_NUM_LOCK; 
        return 0;
    } else if (scancode == KEY_SCROLL_LOCK_PRESS) {
        kbd_flags ^= KEY_SCROLL_LOCK; 
        return 0;
    }
    if (scancode < 128) {
        if ((kbd_flags & KEY_SHIFT) || (kbd_flags & KEY_CAPS)) {
            if (kbd_flags & KEY_CAPS) {
                if ((kbd_us[scancode] >= 'a' && kbd_us[scancode] <= 'z') ||
                    (kbd_us[scancode] >= 'A' && kbd_us[scancode] <= 'Z')) {
                    if (kbd_flags & KEY_SHIFT) {
                        return kbd_us[scancode]; 
                    } else {
                        return kbd_us_shift[scancode]; 
                    }
                }
            }
            if (kbd_flags & KEY_SHIFT) {
                return kbd_us_shift[scancode];
            }
        }
        return kbd_us[scancode];
    }
    return 0;
}
typedef struct {
    char name[MAX_FILENAME];
    char content[MAX_CONTENT];
    unsigned char type;
    unsigned char permissions; 
    int parent_dir;           
    int size;                 
    int created_time;         
} FileEntry;
void print_entry_colored(FileEntry* entry);
int mkdir_p(const char* path);
int resolve_path_full(const char* path, int want_file);
FileEntry filesystem[MAX_FILES + MAX_DIRS];
int fs_entry_count = 0;
int current_dir = 0;

int use_fat32 = 0;
unsigned int fat32_current_cluster = 0; 
void init_filesystem() {
    strcpy(filesystem[0].name, "/");
    filesystem[0].content[0] = '\0';
    filesystem[0].type = TYPE_DIR;
    filesystem[0].permissions = 7; 
    filesystem[0].parent_dir = -1; 
    filesystem[0].size = 0;
    filesystem[0].created_time = 0;
    strcpy(filesystem[1].name, "home");
    filesystem[1].content[0] = '\0';
    filesystem[1].type = TYPE_DIR;
    filesystem[1].permissions = 7; 
    filesystem[1].parent_dir = 0;  
    filesystem[1].size = 0;
    filesystem[1].created_time = 0;
    strcpy(filesystem[2].name, "welcome.txt");
    strcpy(filesystem[2].content, "Welcome to LiteOS with advanced filesystem!");
    filesystem[2].type = TYPE_FILE;
    filesystem[2].permissions = 3; 
    filesystem[2].parent_dir = 0;  
    filesystem[2].size = 43;
    filesystem[2].created_time = 0;
    strcpy(filesystem[3].name, "readme.txt");
    strcpy(filesystem[3].content, "This is a simple OS with a hierarchical filesystem.");
    filesystem[3].type = TYPE_FILE;
    filesystem[3].permissions = 3; 
    filesystem[3].parent_dir = 1;  
    filesystem[3].size = 51;
    filesystem[3].created_time = 0;
    fs_entry_count = 4;
}
void get_current_path(char* buffer) {
    if (current_dir == 0) {
        strcpy(buffer, "/");
        return;
    }
    char temp_path[256] = "";
    int dir_index = current_dir;
    while (dir_index != 0) {
        char temp[MAX_FILENAME + 2];
        strcpy(temp, "/");
        strcat(temp, filesystem[dir_index].name);
        strcat(temp, temp_path);
        strcpy(temp_path, temp);
        dir_index = filesystem[dir_index].parent_dir;
    }
    if (temp_path[0] == '\0') {
        strcpy(buffer, "/");
    } else {
        strcpy(buffer, temp_path);
    }
}
int find_entry(const char* name) {
    for (int i = 0; i < fs_entry_count; i++) {
        if (i < fs_entry_count && filesystem[i].parent_dir == current_dir && 
            my_strncmp(filesystem[i].name, name, MAX_FILENAME) == 0) {
            return i;
        }
    }
    return -1;
}
int create_file(const char* name, const char* content) {
    if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
        return -1; 
    }
    if (find_entry(name) != -1) {
        return -2; 
    }
    strcpy(filesystem[fs_entry_count].name, name);
    strcpy(filesystem[fs_entry_count].content, content);
    filesystem[fs_entry_count].type = TYPE_FILE;
    filesystem[fs_entry_count].permissions = 3; 
    filesystem[fs_entry_count].parent_dir = current_dir;
    filesystem[fs_entry_count].size = 0;
    while (content[filesystem[fs_entry_count].size]) {
        filesystem[fs_entry_count].size++;
    }
    filesystem[fs_entry_count].created_time = 0;
    return fs_entry_count++;
}
int create_directory(const char* name) {
    if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
        return -1; 
    }
    if (find_entry(name) != -1) {
        return -2; 
    }
    strcpy(filesystem[fs_entry_count].name, name);
    filesystem[fs_entry_count].content[0] = '\0';
    filesystem[fs_entry_count].type = TYPE_DIR;
    filesystem[fs_entry_count].permissions = 7; 
    filesystem[fs_entry_count].parent_dir = current_dir;
    filesystem[fs_entry_count].size = 0;
    filesystem[fs_entry_count].created_time = 0;
    return fs_entry_count++;
}
void delay() {
    for (volatile int i = 0; i < 1000000; i++);
}
void shutdown() {
    shell_print_string("\nShutting down the system...\n");
    outw(0x604, 0x2000);
    delay();
    outw(0x4004, 0x3400);
    delay();
    outw(0x600, 0x34);
    delay();
    outw(0xB004, 0x2000);
    delay();
    shell_print_string("Failed to shut down. System halted.\n");
    while (1) {
        asm volatile ("hlt");
    }
}
void process_cmd(char* cmd);
void readline(char* buffer, int max_len);
int find_matching_commands(const char* prefix, char matches[][128], int max_matches);
int find_matching_files(const char* prefix, char matches[][128], int max_matches, int only_files);
void editor_display();
void editor_init(int file_index);
void editor_process_key(int key);
void editor_run(char* filename);
void kernel_main();
/* الدالة الرئيسية للنواة */
int main() {
    // Call kernel_main to start the system
    kernel_main();
    return 0;
}

void kernel_main() {
    clear_screen();
    set_color(LIGHT_GREEN, BLACK);
    shell_print_string("Welcome to SkyOS! [v4.1 - Advanced Operating System]\n");
    set_color(WHITE, BLACK);
    shell_print_string("Type 'help' for a list of commands.\n");
    shell_print_string("Enhanced keyboard & FAT32 support. Use 'fat32 init' to initialize FAT32 FS.\n\n");
    init_keyboard();   
    init_filesystem();
    
    shell_print_colored("Initializing FAT32 file system...\n", COLOR_INFO, BLACK);
    if (fat32_init() == 0) {
        shell_print_colored("✓ FAT32 initialized successfully (512KB)\n", COLOR_SUCCESS, BLACK);
    } else {
        shell_print_colored("✗ FAT32 initialization failed\n", COLOR_ERROR, BLACK);
    }
    shell_print_char('\n'); 
    char cmd_buffer[128];
    char current_path[256];
    while (1) {
        get_current_path(current_path);
        shell_print_colored("SkyOS", COLOR_SUCCESS, BLACK);
        shell_print_colored(" ", WHITE, BLACK);
        shell_print_colored(current_path, COLOR_DIR, BLACK);
        shell_print_colored(" > ", COLOR_WARNING, BLACK);
        set_color(WHITE, BLACK);
        readline(cmd_buffer, sizeof(cmd_buffer));
        if (cmd_buffer[0] != '\0') {
            process_cmd(cmd_buffer);
        }
    }
}
void process_cmd(char* cmd) {
    char* saveptr;
    char* command = strtok_r(cmd, " ", &saveptr);
    if (command == NULL) { 
        return;
    }
    if (my_strncmp(command, "clear", 5) == 0) {
        clear_screen();
    } else if (my_strncmp(command, "help", 4) == 0) {
        char* option = strtok_r(NULL, " ", &saveptr);
        if (option && my_strncmp(option, "full", 4) == 0) {
            show_full_help(); 
        } else if (option) {
            show_command_help(option);
        } else {
            show_quick_help(); 
        }
    } else if (my_strncmp(command, "ls", 2) == 0) {
        char* path = strtok_r(NULL, " ", &saveptr);
        
        if (use_fat32) {
            unsigned int target_cluster = fat32_current_cluster;
            
            if (path) {
                FAT32_DirEntry* entry = fat32_find_file(path);
                if (!entry) {
                    shell_print_colored("Error: ", COLOR_ERROR, BLACK);
                    shell_print_colored("Directory not found: ", COLOR_ERROR, BLACK);
                    shell_print_colored(path, COLOR_WARNING, BLACK);
                    shell_print_char('\n');
                    return;
                }
                target_cluster = ((unsigned int)entry->cluster_high << 16) | entry->cluster_low;
            }
            
            fat32_list_directory(target_cluster);
            shell_print_char('\n');
            return;
        }
        
        int target_dir = current_dir;
        
        if (path) {
            target_dir = resolve_path_full(path, 0);
            if (target_dir == -1) {
                shell_print_colored("Error: ", COLOR_ERROR, BLACK);
                shell_print_colored("Directory not found: ", COLOR_ERROR, BLACK);
                shell_print_colored(path, COLOR_WARNING, BLACK);
                shell_print_char('\n');
                return;
            }
        }
        
        int count = 0;
        int maxlen = 0;
        int indices[MAX_FILES+MAX_DIRS];
        for (int i = 0; i < fs_entry_count; i++) {
            if (filesystem[i].parent_dir == target_dir) {
                indices[count++] = i;
                int l = my_strlen(filesystem[i].name);
                if (filesystem[i].type == TYPE_DIR) l++;
                if (l > maxlen) maxlen = l;
            }
        }
        if (count == 0) {
            shell_print_string("Directory is empty\n");
        } else {
            int cols = VGA_WIDTH / (maxlen+3);
            if (cols < 1) cols = 1;
            for (int row = 0; row < (count+cols-1)/cols; row++) {
                for (int col = 0; col < cols; col++) {
                    int idx = row + col*((count+cols-1)/cols);
                    if (idx < count) {
                        FileEntry* entry = &filesystem[indices[idx]];
                        print_entry_colored(entry);
                        int l = strlen(entry->name);
                        if (entry->type == TYPE_DIR) l++;
                        for (int s = l; s < maxlen+2; s++) shell_print_char(' ');
                    }
                }
                shell_print_char('\n');
            }
        }
        return;
    } else if (strncmp(command, "cd", 2) == 0) {
        char* dirname = strtok_r(NULL, " ", &saveptr);
        int new_dir = 0;
        if (!dirname) {
            new_dir = 0;
        } else {
            new_dir = resolve_path_full(dirname, 0);
        }
        if (new_dir == -1) {
            shell_print_colored("Error: ", COLOR_ERROR, BLACK);
            shell_print_colored("Directory not found: ", COLOR_ERROR, BLACK);
            shell_print_colored(dirname, COLOR_WARNING, BLACK);
            shell_print_char('\n');
        } else {
            current_dir = new_dir;
            shell_print_colored("Changed to: ", COLOR_SUCCESS, BLACK);
            char path[256];
            get_current_path(path);
            shell_print_colored(path, COLOR_DIR, BLACK);
            shell_print_char('\n');
        }
        return;
    } else if (strncmp(command, "mkdir", 5) == 0) {
        char* dirname = strtok_r(NULL, " ", &saveptr);
        if (!dirname) {
            shell_print_colored("Usage: ", COLOR_INFO, BLACK);
            shell_print_colored("mkdir <dirname>\n", COLOR_WARNING, BLACK);
            return;
        }
        
        if (use_fat32) {
            if (fat32_create_directory(dirname, fat32_current_cluster) == 0) {
                shell_print_colored("Success: ", COLOR_SUCCESS, BLACK);
                shell_print_colored("FAT32 directory created: ", COLOR_SUCCESS, BLACK);
                shell_print_colored(dirname, COLOR_DIR, BLACK);
                shell_print_char('\n');
            } else {
                shell_print_colored("Error: ", COLOR_ERROR, BLACK);
                shell_print_colored("Failed to create FAT32 directory\n", COLOR_ERROR, BLACK);
            }
            return;
        }
        
        int result = mkdir_p(dirname);
        if (result == -1) {
            shell_print_colored("Error: ", COLOR_ERROR, BLACK);
            shell_print_colored("Filesystem is full!\n", COLOR_ERROR, BLACK);
        } else if (result == -2) {
            shell_print_colored("Warning: ", COLOR_WARNING, BLACK);
            shell_print_colored("Directory already exists: ", COLOR_WARNING, BLACK);
            shell_print_colored(dirname, COLOR_DIR, BLACK);
            shell_print_char('\n');
        } else if (result == -3) {
            shell_print_colored("Error: ", COLOR_ERROR, BLACK);
            shell_print_colored("File exists with same name: ", COLOR_ERROR, BLACK);
            shell_print_colored(dirname, COLOR_WARNING, BLACK);
            shell_print_char('\n');
        } else {
            shell_print_colored("Success: ", COLOR_SUCCESS, BLACK);
            shell_print_colored("Directory created: ", COLOR_SUCCESS, BLACK);
            shell_print_colored(dirname, COLOR_DIR, BLACK);
            shell_print_char('\n');
        }
        return;
    } else if (strncmp(command, "pwd", 3) == 0) {
        char path[256];
        get_current_path(path);
        shell_print_string(path);
        shell_print_string("\n");
    } else if (strncmp(command, "cat", 3) == 0) {
        char* filename = strtok_r(NULL, " ", &saveptr);
        if (!filename) {
            shell_print_string("Usage: cat <filename>\n");
            return;
        }
        int file_index = resolve_path_full(filename, 1);
        if (file_index == -1) {
            shell_print_colored("Error: ", COLOR_ERROR, BLACK);
            shell_print_colored("File not found: ", COLOR_ERROR, BLACK);
            shell_print_colored(filename, COLOR_WARNING, BLACK);
            shell_print_char('\n');
        } else if (filesystem[file_index].type != TYPE_FILE) {
            shell_print_colored("Error: ", COLOR_ERROR, BLACK);
            shell_print_colored("Not a file: ", COLOR_ERROR, BLACK);
            shell_print_string(filename);
            shell_print_string("\n");
        } else if (!(filesystem[file_index].permissions & 1)) {
            shell_print_string("Permission denied: ");
            shell_print_string(filename);
            shell_print_string(" is not readable\n");
        } else {
            shell_print_string(filesystem[file_index].content);
            shell_print_string("\n");
        }
        return;
    } else if (strncmp(command, "write", 5) == 0) {
        char* filename = strtok_r(NULL, " ", &saveptr);
        char* content_start = strchr(saveptr, '"');
        if (filename && content_start) {
            content_start++; 
            char* content_end = strchr(content_start, '"');
            if (content_end) {
                *content_end = '\0'; 
                int file_index = resolve_path_full(filename, 1);
                if (file_index != -1) {
                    if (filesystem[file_index].type != TYPE_FILE) {
                        shell_print_string("Not a file: ");
                        shell_print_string(filename);
                        shell_print_string("\n");
                    } else if (!(filesystem[file_index].permissions & 2)) {
                        shell_print_string("Permission denied: ");
                        shell_print_string(filename);
                        shell_print_string(" is not writable\n");
                    } else {
                        strcpy(filesystem[file_index].content, content_start);
                        filesystem[file_index].size = 0;
                        while (content_start[filesystem[file_index].size]) {
                            filesystem[file_index].size++;
                        }
                    }
                } else {
                    char pathcpy[256]; strcpy(pathcpy, filename);
                    char* lastslash = NULL;
                    for (char* p = pathcpy; *p; p++) if (*p == '/') lastslash = p;
                    int parent_dir = current_dir;
                    char* fname = pathcpy;
                    if (lastslash) {
                        *lastslash = '\0';
                        parent_dir = resolve_path_full(pathcpy, 0);
                        fname = lastslash+1;
                    }
                    if (parent_dir == -1) {
                        shell_print_string("Invalid path: ");
                        shell_print_string(filename);
                        shell_print_string("\n");
                    } else {
                        int old_dir = current_dir;
                        current_dir = parent_dir;
                        int result = create_file(fname, content_start);
                        current_dir = old_dir;
                        if (result < 0) {
                            shell_print_string("Filesystem is full!\n");
                        }
                    }
                }
            } else {
                shell_print_string("Write format error: missing closing quote.\n");
            }
        } else {
            shell_print_string("Usage: write <filename> \"content\"\n");
        }
        return;
    } else if (strncmp(command, "rm", 2) == 0) {
        char* filename = strtok_r(NULL, " ", &saveptr);
        if (!filename) {
            shell_print_string("Usage: rm <filename>\n");
            return;
        }
        int file_index = resolve_path_full(filename, 1);
        if (file_index == -1) {
            shell_print_string("File not found: ");
            shell_print_string(filename);
            shell_print_string("\n");
        } else if (filesystem[file_index].type == TYPE_DIR) {
            shell_print_string("Cannot remove directory: ");
            shell_print_string(filename);
            shell_print_string("\n");
        } else {
        filesystem[file_index] = filesystem[fs_entry_count - 1];
        fs_entry_count--;
        shell_print_string("File removed\n");
    }
    return;
    } else if (strncmp(command, "chmod", 5) == 0) {
        char* filename = strtok_r(NULL, " ", &saveptr);
        char* perm_str = strtok_r(NULL, " ", &saveptr);
        if (!filename || !perm_str) {
            shell_print_string("Usage: chmod <filename> <perm>\n");
            shell_print_string("Permissions: 1=read, 2=write, 4=execute\n");
            return;
        }
        int perm = 0;
        for (int i = 0; perm_str[i]; i++) {
            if (perm_str[i] >= '0' && perm_str[i] <= '7') {
                perm = perm_str[i] - '0';
                break;
            }
        }
        int file_index = resolve_path_full(filename, 1);
        if (file_index == -1) {
            shell_print_string("File not found: ");
            shell_print_string(filename);
            shell_print_string("\n");
        } else {
            filesystem[file_index].permissions = perm;
            shell_print_string("Permissions changed\n");
        }
        return;
    } else if (strncmp(command, "color", 5) == 0) {
        char* fg_str = strtok_r(NULL, " ", &saveptr);
        char* bg_str = strtok_r(NULL, " ", &saveptr);
        if (!fg_str || !bg_str) {
            shell_print_string("Usage: color <fg> <bg>\n");
            shell_print_string("Colors: 0=black, 1=blue, 2=green, 4=red, 7=light gray, 15=white\n");
            return;
        }
        int fg = 0, bg = 0;
        for (int i = 0; fg_str[i]; i++) {
            if (fg_str[i] >= '0' && fg_str[i] <= '9') {
                fg = fg * 10 + (fg_str[i] - '0');
            }
        }
        for (int i = 0; bg_str[i]; i++) {
            if (bg_str[i] >= '0' && bg_str[i] <= '9') {
                bg = bg * 10 + (bg_str[i] - '0');
            }
        }
        if (fg > 15) fg = 15;
        if (bg > 15) bg = 15;
        set_color(fg, bg);
        shell_print_string("Color changed\n");
    } else if (strncmp(command, "sysinfo", 7) == 0) {
        shell_print_colored("SkyOS System Information\n", LIGHT_GREEN, BLACK);
        shell_print_string("Version: 0.3.1\n");
        shell_print_string("Memory: 640KB Base, 64KB Extended\n");
        shell_print_string("Display: VGA Text Mode (80x25)\n");
        shell_print_string("Filesystem: In-memory Hierarchical FS\n");
        shell_print_string("Files: ");
        int files = 0, dirs = 0;
        for (int i = 0; i < fs_entry_count; i++) {
            if (i < fs_entry_count) {
                if (filesystem[i].type == TYPE_FILE) {
                    files++;
                } else {
                    dirs++;
                }
            }
        }
        char num_str[10];
        int idx = 0, temp = files;
        if (temp == 0) {
            num_str[idx++] = '0';
        } else {
            while (temp > 0) {
                num_str[idx++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        num_str[idx] = '\0';
        for (int j = 0; j < idx / 2; j++) {
            char temp = num_str[j];
            num_str[j] = num_str[idx - j - 1];
            num_str[idx - j - 1] = temp;
        }
        shell_print_string(num_str);
        shell_print_string(", Directories: ");
        idx = 0;
        temp = dirs;
        if (temp == 0) {
            num_str[idx++] = '0';
        } else {
            while (temp > 0) {
                num_str[idx++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        num_str[idx] = '\0';
        for (int j = 0; j < idx / 2; j++) {
            char temp = num_str[j];
            num_str[j] = num_str[idx - j - 1];
            num_str[idx - j - 1] = temp;
        }
        shell_print_string(num_str);
        shell_print_string("\n");
    } else if (strncmp(command, "fastfetch", 9) == 0) {
        shell_print_colored("                    /\\\n", LIGHT_CYAN, BLACK);
        shell_print_colored("                   /  \\\n", LIGHT_CYAN, BLACK);
        shell_print_colored("                  /    \\\n", LIGHT_CYAN, BLACK);
        shell_print_colored("                 /      \\\n", LIGHT_CYAN, BLACK);
        shell_print_colored("                /   ,,   \\\n", LIGHT_CYAN, BLACK);
        shell_print_colored("               /   |  |   \\\n", LIGHT_CYAN, BLACK);
        shell_print_colored("              /_-''    ''-_\\\n", LIGHT_CYAN, BLACK);
        shell_print_colored("\n", WHITE, BLACK);
        shell_print_colored("OS: ", LIGHT_GREEN, BLACK);
        shell_print_string("SkyOS v4.1\n");
        shell_print_colored("Kernel: ", LIGHT_GREEN, BLACK);
        shell_print_string("SkyOS Kernel v4.1\n");
        shell_print_colored("Architecture: ", LIGHT_GREEN, BLACK);
        shell_print_string("x86 (32-bit)\n");
        shell_print_colored("Memory: ", LIGHT_GREEN, BLACK);
        shell_print_string("640KB Base + 64KB Extended\n");
        shell_print_colored("Display: ", LIGHT_GREEN, BLACK);
        shell_print_string("VGA Text Mode (80x25)\n");
        shell_print_colored("Shell: ", LIGHT_GREEN, BLACK);
        shell_print_string("SkyOS Shell v4.1\n");
        shell_print_colored("Terminal: ", LIGHT_GREEN, BLACK);
        shell_print_string("VGA Console\n");
        shell_print_colored("CPU: ", LIGHT_GREEN, BLACK);
        shell_print_string("Intel 80386+ Compatible\n");
        shell_print_colored("Filesystem: ", LIGHT_GREEN, BLACK);
        shell_print_string("In-Memory Hierarchical FS\n");
        int files = 0, dirs = 0;
        for (int i = 0; i < fs_entry_count; i++) {
            if (filesystem[i].type == TYPE_FILE) {
                files++;
            } else {
                dirs++;
            }
        }
        shell_print_colored("Files: ", LIGHT_GREEN, BLACK);
        char num_str[10];
        int idx = 0, temp = files;
        if (temp == 0) {
            num_str[idx++] = '0';
        } else {
            while (temp > 0) {
                num_str[idx++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        num_str[idx] = '\0';
        for (int j = 0; j < idx / 2; j++) {
            char temp = num_str[j];
            num_str[j] = num_str[idx - j - 1];
            num_str[idx - j - 1] = temp;
        }
        shell_print_string(num_str);
        shell_print_string(" files, ");
        idx = 0;
        temp = dirs;
        if (temp == 0) {
            num_str[idx++] = '0';
        } else {
            while (temp > 0) {
                num_str[idx++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        num_str[idx] = '\0';
        for (int j = 0; j < idx / 2; j++) {
            char temp = num_str[j];
            num_str[j] = num_str[idx - j - 1];
            num_str[idx - j - 1] = temp;
        }
        shell_print_string(num_str);
        shell_print_string(" directories\n");
        shell_print_colored("Uptime: ", LIGHT_GREEN, BLACK);
        shell_print_string("System Ready\n");
        shell_print_colored("Packages: ", LIGHT_GREEN, BLACK);
        shell_print_string("Built-in Commands\n");
        shell_print_colored("Theme: ", LIGHT_GREEN, BLACK);
        shell_print_string("VGA Color Palette\n");
        shell_print_colored("Icons: ", LIGHT_GREEN, BLACK);
        shell_print_string("ASCII Art\n");
    } else if (strncmp(command, "vim", 3) == 0) {
        char* filename = strtok_r(NULL, " ", &saveptr);
        if (!filename) {
            shell_print_string("Usage: vim <filename>\n");
            return;
        }
        int file_index = resolve_path_full(filename, 1);
        if (file_index == -1) {
            char pathcpy[256]; strcpy(pathcpy, filename);
            char* lastslash = NULL;
            for (char* p = pathcpy; *p; p++) if (*p == '/') lastslash = p;
            int parent_dir = current_dir;
            char* fname = pathcpy;
            if (lastslash) {
                *lastslash = '\0';
                parent_dir = resolve_path_full(pathcpy, 0);
                fname = lastslash+1;
            }
            if (parent_dir == -1) {
                shell_print_string("Invalid path: ");
                shell_print_string(filename);
                shell_print_string("\n");
                return;
            }
            int old_dir = current_dir;
            current_dir = parent_dir;
            file_index = create_file(fname, "");
            current_dir = old_dir;
            if (file_index < 0) {
                shell_print_string("Could not create file\n");
                return;
            }
        } else if (filesystem[file_index].type != TYPE_FILE) {
            shell_print_string("Not a file: ");
            shell_print_string(filename);
            shell_print_string("\n");
            return;
        } else if (!(filesystem[file_index].permissions & 2)) {
            shell_print_string("Permission denied: ");
            shell_print_string(filename);
            shell_print_string(" is not writable\n");
            return;
        }
        editor_run(filesystem[file_index].name);
        return;
    } else if (strncmp(command, "run", 3) == 0) {
        char* filename = strtok_r(NULL, " ", &saveptr);
        if (!filename) {
            shell_print_string("Usage: run <filename.c>\n");
            return;
        }
        int file_index = resolve_path_full(filename, 1);
        if (file_index == -1) {
            shell_print_string("File not found: ");
            shell_print_string(filename);
            shell_print_string("\n");
            return;
        }
        if (filesystem[file_index].type != TYPE_FILE) {
            shell_print_string("Not a file: ");
            shell_print_string(filename);
            shell_print_string("\n");
            return;
        }
        int len = 0;
        while (filename[len] != '\0') len++;
        if (len < 3 || filename[len-2] != '.' || filename[len-1] != 'c') {
            shell_print_string("Not a C source file.\n");
            return;
        }
        char* content = filesystem[file_index].content;
        int i = 0;
        while (content[i] != '\0') {
            if (content[i] == 'p' && content[i+1] == 'r' && content[i+2] == 'i' && content[i+3] == 'n' && content[i+4] == 't' && content[i+5] == 'f' && content[i+6] == '(' && content[i+7] == '"') {
                i += 8;
                while (content[i] != '\0' && content[i] != '"') {
                    shell_print_char(content[i]);
                    i++;
                }
                shell_print_char('\n');
            } else {
                i++;
            }
        }
        return;
    } else if (strncmp(command, "touch", 5) == 0) {
        char* filename = strtok_r(NULL, " ", &saveptr);
        if (!filename) {
            shell_print_colored("Usage: ", COLOR_INFO, BLACK);
            shell_print_colored("touch <filename>\n", COLOR_WARNING, BLACK);
            return;
        }
        
        if (use_fat32) {
            if (fat32_create_file(filename, fat32_current_cluster) == 0) {
                shell_print_colored("Success: ", COLOR_SUCCESS, BLACK);
                shell_print_colored("FAT32 file created: ", COLOR_SUCCESS, BLACK);
                shell_print_colored(filename, COLOR_FILE, BLACK);
                shell_print_char('\n');
            } else {
                shell_print_colored("Error: ", COLOR_ERROR, BLACK);
                shell_print_colored("Failed to create FAT32 file\n", COLOR_ERROR, BLACK);
            }
            return;
        }
        
        if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
            shell_print_colored("Error: ", COLOR_ERROR, BLACK);
            shell_print_colored("Filesystem is full!\n", COLOR_ERROR, BLACK);
            return;
        }
        
        if (resolve_path_full(filename, 1) != -1) {
            shell_print_colored("Warning: ", COLOR_WARNING, BLACK);
            shell_print_colored("File already exists: ", COLOR_WARNING, BLACK);
            shell_print_colored(filename, COLOR_FILE, BLACK);
            shell_print_char('\n');
            return;
        }
        
        strcpy(filesystem[fs_entry_count].name, filename);
        filesystem[fs_entry_count].content[0] = '\0';
        filesystem[fs_entry_count].type = TYPE_FILE;
        filesystem[fs_entry_count].permissions = 6;
        filesystem[fs_entry_count].parent_dir = current_dir;
        filesystem[fs_entry_count].size = 0;
        filesystem[fs_entry_count].created_time = fs_entry_count;
        fs_entry_count++;
        
        shell_print_colored("Success: ", COLOR_SUCCESS, BLACK);
        shell_print_colored("File created: ", COLOR_SUCCESS, BLACK);
        shell_print_colored(filename, COLOR_FILE, BLACK);
        shell_print_char('\n');
        return;
    } else if (strncmp(command, "shutdown", 8) == 0) {
        shell_print_colored("Shutting down system...\n", COLOR_WARNING, BLACK);
        shutdown();
    } else if (strncmp(command, "tree", 4) == 0) {
        shell_print_colored("Directory tree:\n", COLOR_INFO, BLACK);
        if (use_fat32) {
            shell_print_colored("FAT32 tree view not implemented yet\n", COLOR_WARNING, BLACK);
        } else {
            print_tree(0, 0);
        }
        return;
    } else if (strncmp(command, "fat32", 5) == 0) {
        char* subcommand = strtok_r(NULL, " ", &saveptr);
        if (!subcommand) {
            shell_print_colored("Usage: fat32 <init|info|switch>\n", COLOR_INFO, BLACK);
            return;
        }
        
        if (strncmp(subcommand, "init", 4) == 0) {
            shell_print_colored("Initializing FAT32 file system...\n", COLOR_INFO, BLACK);
            if (fat32_init() == 0) {
                shell_print_colored("Success: ", COLOR_SUCCESS, BLACK);
                shell_print_colored("FAT32 file system initialized (512KB)\n", COLOR_SUCCESS, BLACK);
            } else {
                shell_print_colored("Error: ", COLOR_ERROR, BLACK);
                shell_print_colored("Failed to initialize FAT32\n", COLOR_ERROR, BLACK);
            }
        } else if (strncmp(subcommand, "info", 4) == 0) {
            shell_print_colored("File System Information:\n", COLOR_INFO, BLACK);
            shell_print_colored("Current FS: ", COLOR_INFO, BLACK);
            if (use_fat32) {
                shell_print_colored("FAT32\n", COLOR_SUCCESS, BLACK);
                shell_print_colored("Cluster Size: 4KB\n", COLOR_INFO, BLACK);
                shell_print_colored("Sector Size: 512 bytes\n", COLOR_INFO, BLACK);
                shell_print_colored("Volume Label: SKYOS\n", COLOR_INFO, BLACK);
            } else {
                shell_print_colored("Simple RAM FS\n", COLOR_WARNING, BLACK);
                shell_print_colored("Max Files: 32\n", COLOR_INFO, BLACK);
                shell_print_colored("Max Dirs: 16\n", COLOR_INFO, BLACK);
            }
        } else if (strncmp(subcommand, "switch", 6) == 0) {
            if (use_fat32) {
                use_fat32 = 0;
                current_dir = 0;
                shell_print_colored("Switched to: ", COLOR_SUCCESS, BLACK);
                shell_print_colored("Simple RAM FS\n", COLOR_SUCCESS, BLACK);
            } else {
                use_fat32 = 1;
                fat32_current_cluster = fat32_get_root_cluster();
                shell_print_colored("Switched to: ", COLOR_SUCCESS, BLACK);
                shell_print_colored("FAT32 FS\n", COLOR_SUCCESS, BLACK);
            }
        } else {
            shell_print_colored("Unknown subcommand: ", COLOR_ERROR, BLACK);
            shell_print_colored(subcommand, COLOR_WARNING, BLACK);
            shell_print_char('\n');
        }
        return;
    } else if (strncmp(command, "debug", 5) == 0) {
        char* path = strtok_r(NULL, " ", &saveptr);
        if (!path) {
            shell_print_string("Usage: debug <path>\n");
            return;
        }
        
        shell_print_colored("Debug info for: ", COLOR_INFO, BLACK);
        shell_print_colored(path, COLOR_WARNING, BLACK);
        shell_print_char('\n');
        
        shell_print_colored("Current dir: ", COLOR_INFO, BLACK);
        char current_path[256];
        get_current_path(current_path);
        shell_print_colored(current_path, COLOR_DIR, BLACK);
        shell_print_char('\n');
        
        shell_print_colored("Searching for file: ", COLOR_INFO, BLACK);
        int file_result = resolve_path_full(path, 1);
        if (file_result == -1) {
            shell_print_colored("NOT FOUND\n", COLOR_ERROR, BLACK);
        } else {
            shell_print_colored("FOUND at index ", COLOR_SUCCESS, BLACK);
            char index_str[10];
            index_str[0] = '0' + file_result;
            index_str[1] = '\0';
            shell_print_colored(index_str, COLOR_SUCCESS, BLACK);
            shell_print_colored(" - ", WHITE, BLACK);
            shell_print_colored(filesystem[file_result].name, COLOR_FILE, BLACK);
            shell_print_char('\n');
        }
        
        shell_print_colored("Searching for dir: ", COLOR_INFO, BLACK);
        int dir_result = resolve_path_full(path, 0);
        if (dir_result == -1) {
            shell_print_colored("NOT FOUND\n", COLOR_ERROR, BLACK);
        } else {
            shell_print_colored("FOUND at index ", COLOR_SUCCESS, BLACK);
            char index_str[10];
            index_str[0] = '0' + dir_result;
            index_str[1] = '\0';
            shell_print_colored(index_str, COLOR_SUCCESS, BLACK);
            shell_print_colored(" - ", WHITE, BLACK);
            shell_print_colored(filesystem[dir_result].name, COLOR_DIR, BLACK);
            shell_print_char('\n');
        }
        
        shell_print_colored("All files in filesystem:\n", COLOR_INFO, BLACK);
        for (int i = 0; i < fs_entry_count; i++) {
            char index_str[10];
            index_str[0] = '0' + (i / 10);
            index_str[1] = '0' + (i % 10);
            index_str[2] = '\0';
            shell_print_colored(index_str, COLOR_WARNING, BLACK);
            shell_print_colored(": ", WHITE, BLACK);
            shell_print_colored(filesystem[i].name, 
                filesystem[i].type == TYPE_DIR ? COLOR_DIR : COLOR_FILE, BLACK);
            shell_print_colored(" (parent: ", WHITE, BLACK);
            char parent_str[10];
            parent_str[0] = '0' + filesystem[i].parent_dir;
            parent_str[1] = '\0';
            shell_print_colored(parent_str, COLOR_WARNING, BLACK);
            shell_print_colored(")\n", WHITE, BLACK);
        }
        return;
    } else {
        shell_print_colored("Error: ", COLOR_ERROR, BLACK);
        shell_print_colored("Unknown command: ", COLOR_ERROR, BLACK);
        shell_print_colored(command, COLOR_WARNING, BLACK);
        shell_print_colored("\nType ", WHITE, BLACK);
        shell_print_colored("help", COLOR_INFO, BLACK);
        shell_print_colored(" for available commands.\n", WHITE, BLACK);
    }
}
void readline(char* buffer, int max_len) {
    int index = 0;
    int cursor_pos = 0;
    buffer[0] = '\0';
    while (1) {
        int key = get_char();
        if (key == 0) continue;
        if (key >= F1_CODE && key <= F12_CODE) {
            switch (key) {
                case F1_CODE:
                    shell_print_char('\n');
                    show_quick_help();
                    break;
                case F2_CODE:
                    shell_print_string("\n[F2: Clear] ");
                    clear_screen();
                    break;
                case F3_CODE:
                    shell_print_string("\n[F3: List] ");
                    break;
                default:
                    shell_print_string("\n[Function key pressed]\n");
                    break;
            }
            continue;
        }
        if (key == 3) { 
            shell_print_string("^C\n");
            buffer[0] = '\0';
            return;
        } else if (key == 4) { 
            shell_print_string("^D\n");
            buffer[0] = '\0';
            return;
        } else if (key == 12) { 
            clear_screen();
            continue;
        } else if (key == 26) { 
            shell_print_string("^Z\n");
            continue;
        }
        if (key == '\n') {
            buffer[index] = '\0';
            shell_print_char('\n');
            return;
        } else if (key == 0x08) { 
            if (cursor_pos > 0) {
                for (int i = cursor_pos - 1; i < index - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                index--;
                cursor_pos--;
                buffer[index] = '\0';
                shell_print_char(0x08);
                for (int i = cursor_pos; i < index; i++) shell_print_char(buffer[i]);
                shell_print_char(' ');
                for (int i = 0; i <= index - cursor_pos; i++) shell_print_char(0x08);
            }
        } else if (key == KEY_DEL_CODE) { 
            if (cursor_pos < index) {
                for (int i = cursor_pos; i < index - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                index--;
                buffer[index] = '\0';
                for (int i = cursor_pos; i < index; i++) shell_print_char(buffer[i]);
                shell_print_char(' ');
                for (int i = 0; i <= index - cursor_pos; i++) shell_print_char(0x08);
            }
        } else if (key == ARROW_LEFT) {
            if (cursor_pos > 0) {
                cursor_pos--;
                shell_print_char(0x08); 
            }
        } else if (key == ARROW_RIGHT) {
            if (cursor_pos < index) {
                cursor_pos++;
                shell_print_char(buffer[cursor_pos - 1]); 
            }
        } else if (key == KEY_HOME_CODE) { 
            while (cursor_pos > 0) {
                cursor_pos--;
                shell_print_char(0x08);
            }
        } else if (key == KEY_END_CODE) { 
            while (cursor_pos < index) {
                shell_print_char(buffer[cursor_pos]);
                cursor_pos++;
            }
        } else if (key == '\t') { 
            buffer[index] = '\0';
            int word_start = index;
            while (word_start > 0 && buffer[word_start-1] != ' ') {
                word_start--;
            }
            char prefix[128];
            int prefix_len = 0;
            for (int i = word_start; i < index; i++) {
                prefix[prefix_len++] = buffer[i];
            }
            prefix[prefix_len] = '\0';
            char matches[128][128]; 
            int match_count = 0;
            if (word_start == 0) {
                match_count = find_matching_commands(prefix, matches, 128);
            } else {
                match_count = find_matching_files(prefix, matches, 128, 0); 
            }
            if (match_count == 1) {
                int match_len = strlen(matches[0]);
                for (int i = prefix_len; i < match_len && word_start + i < max_len - 1; i++) {
                    buffer[word_start + i] = matches[0][i];
                    shell_print_char(matches[0][i]);
                    index = word_start + i + 1;
                }
                buffer[index] = '\0';
            } else if (match_count > 1) {
                int common = prefix_len;
                int done = 0;
                while (!done) {
                    char c = matches[0][common];
                    if (c == '\0') break;
                    for (int i = 1; i < match_count; i++) {
                        if (matches[i][common] != c) {
                            done = 1;
                            break;
                        }
                    }
                    if (!done) {
                        buffer[word_start + common] = c;
                        shell_print_char(c);
                        common++;
                        index = word_start + common;
                    }
                }
                buffer[index] = '\0';
                if (common == prefix_len) {
                    shell_print_char('\n');
                    for (int i = 0; i < match_count; i++) {
                        shell_print_string(matches[i]);
                        shell_print_string("  ");
                    }
                    shell_print_char('\n');
                    char current_path[256];
                    get_current_path(current_path);
                    shell_print_colored("SkyOS", COLOR_SUCCESS, BLACK);
                    shell_print_colored(" ", WHITE, BLACK);
                    shell_print_colored(current_path, COLOR_DIR, BLACK);
                    shell_print_colored(" > ", COLOR_WARNING, BLACK);
                    set_color(WHITE, BLACK);
                    for (int i = 0; i < index; i++) {
                        shell_print_char(buffer[i]);
                    }
                }
            }
        } else if (index < max_len - 1 && key >= 32 && key <= 126) {
            for (int i = index; i > cursor_pos; i--) buffer[i] = buffer[i-1];
            buffer[cursor_pos] = key;
            index++;
            cursor_pos++;
            for (int i = cursor_pos-1; i < index; i++) shell_print_char(buffer[i]);
            for (int i = 0; i < index - cursor_pos; i++) shell_print_char(0x08);
        }
    }
}
char editor_cmdline[32] = "";
int editor_cmdline_active = 0;
typedef struct {
    int should_exit;
} EditorState;
EditorState editor_state = {0};
void editor_display() {
    clear_screen();
    set_color(LIGHT_GREEN, BLACK);
    shell_print_string("Press Enter to start editing\n");
    set_color(WHITE, BLACK);
    int len = 0, line = 1, col = 0, cur_line = 1, cur_col = 0;
    int cursor_pos = editor_cursor;
    for (int i = 0; i < cursor_pos; i++) {
        if (editor_buffer[i] == '\n') {
            cur_line++;
            cur_col = 0;
        } else {
            cur_col++;
        }
    }
    int i = 0, screen_line = 0;
    while (editor_buffer[i] != '\0' && screen_line < VGA_HEIGHT-3) { 
        if (line == cur_line) {
            set_color(WHITE, BLUE);
        } else {
            set_color(WHITE, BLACK);
        }
        col = 0;
        while (editor_buffer[i] != '\0' && editor_buffer[i] != '\n' && col < VGA_WIDTH) {
            if (line == cur_line && col == cur_col) {
                set_color(YELLOW, RED);
                shell_print_char(editor_buffer[i]);
                set_color(WHITE, BLUE);
            } else {
                shell_print_char(editor_buffer[i]);
            }
            i++; col++;
        }
        if (editor_buffer[i] == '\n') { shell_print_char('\n'); i++; line++; screen_line++; }
        else if (editor_buffer[i] == '\0') { shell_print_char('\n'); screen_line++; break; }
    }
    set_color(DARK_GRAY, BLACK);
    while (screen_line < VGA_HEIGHT-3) {
        shell_print_char('~');
        shell_print_char('\n');
        screen_line++;
    }
    set_color(BLACK, LIGHT_CYAN);
    char status[80];
    int sl = cur_line, sc = cur_col, stidx = 0;
    if (editor_file_index >= 0) {
        const char* fn = filesystem[editor_file_index].name;
        int j = 0; while (fn[j] && stidx < 30) status[stidx++] = fn[j++];
    } else {
        status[stidx++] = '['; status[stidx++] = 'N'; status[stidx++] = 'o'; status[stidx++] = ' '; status[stidx++] = 'N'; status[stidx++] = 'a'; status[stidx++] = 'm'; status[stidx++] = 'e'; status[stidx++] = ']';
    }
    status[stidx++] = ' ';
    if (editor_mode == EDITOR_MODE_NORMAL) {
        status[stidx++] = '-'; status[stidx++] = 'N'; status[stidx++] = 'O'; status[stidx++] = 'R'; status[stidx++] = 'M'; status[stidx++] = 'A'; status[stidx++] = 'L'; status[stidx++] = '-';
    } else {
        status[stidx++] = '-'; status[stidx++] = 'I'; status[stidx++] = 'N'; status[stidx++] = 'S'; status[stidx++] = 'E'; status[stidx++] = 'R'; status[stidx++] = 'T'; status[stidx++] = '-';
    }
    status[stidx++] = ' ';
    status[stidx++] = 'L'; status[stidx++] = ':';
    if (sl > 99) status[stidx++] = '0' + (sl/100);
    if (sl > 9) status[stidx++] = '0' + ((sl/10)%10);
    status[stidx++] = '0' + (sl%10);
    status[stidx++] = ' ';
    status[stidx++] = 'C'; status[stidx++] = ':';
    if (sc > 99) status[stidx++] = '0' + (sc/100);
    if (sc > 9) status[stidx++] = '0' + ((sc/10)%10);
    status[stidx++] = '0' + (sc%10);
    status[stidx] = '\0';
    shell_print_string(status);
    for (int i = stidx; i < VGA_WIDTH; i++) shell_print_char(' ');
    set_color(WHITE, BLACK);
    cursor_y = VGA_HEIGHT - 1;
    cursor_x = 0;
    if (editor_cmdline_active) {
        set_color(WHITE, BLACK);
        shell_print_char(':');
        shell_print_string(editor_cmdline);
        for (int i = 1 + strlen(editor_cmdline); i < VGA_WIDTH; i++) shell_print_char(' ');
    } else {
        for (int i = 0; i < VGA_WIDTH; i++) shell_print_char(' ');
    }
    set_color(WHITE, BLACK);
    update_cursor(0, VGA_HEIGHT - 1);
}
void editor_init(int file_index) {
    editor_file_index = file_index;
    editor_mode = EDITOR_MODE_NORMAL;
    editor_cursor = 0;
    if (file_index >= 0) {
        strcpy(editor_buffer, filesystem[file_index].content);
    } else {
        editor_buffer[0] = '\0';
    }
    editor_display();
}
void editor_process_key(int key) {
    if (key >= F1_CODE && key <= F12_CODE) {
        switch (key) {
            case F1_CODE: 
                clear_screen();
                shell_print_string("=== VIM-like Editor Help ===\n");
                shell_print_string("ESC: Switch between Normal/Insert mode\n");
                shell_print_string("Arrow keys: Navigate cursor\n");
                shell_print_string("Home/End: Go to line beginning/end\n");
                shell_print_string("Page Up/Down: Navigate by pages\n");
                shell_print_string("Delete: Delete character at cursor\n");
                shell_print_string("In Normal mode: :x (save & quit), :q (quit)\n");
                shell_print_string("Ctrl+S: Save file\n\n");
                shell_print_string("Press any key to continue...");
                while (get_char() == 0);
                editor_display();
                return;
            case F2_CODE: 
                if (editor_file_index >= 0) {
                    strcpy(filesystem[editor_file_index].content, editor_buffer);
                    filesystem[editor_file_index].size = strlen(editor_buffer);
                }
                editor_display();
                return;
            case F3_CODE: 
                editor_display();
                return;
        }
        return;
    }
    if (key == 19) { 
        if (editor_file_index >= 0) {
            strcpy(filesystem[editor_file_index].content, editor_buffer);
            filesystem[editor_file_index].size = strlen(editor_buffer);
        }
        editor_display();
        return;
    }
    if (editor_cmdline_active) {
        int len = strlen(editor_cmdline);
        if (key == '\n') {
            editor_cmdline_active = 0;
            if (strcmp(editor_cmdline, "x") == 0 || strcmp(editor_cmdline, "wq") == 0) {
                if (editor_file_index >= 0) {
                    strcpy(filesystem[editor_file_index].content, editor_buffer);
                    filesystem[editor_file_index].size = strlen(editor_buffer);
                }
                editor_cmdline[0] = '\0';
                editor_state.should_exit = 1;
                return;
            } else if (strcmp(editor_cmdline, "q") == 0) {
                editor_cmdline[0] = '\0';
                editor_state.should_exit = 1;
                return;
            } else if (strcmp(editor_cmdline, "w") == 0) {
                if (editor_file_index >= 0) {
                    strcpy(filesystem[editor_file_index].content, editor_buffer);
                    filesystem[editor_file_index].size = strlen(editor_buffer);
                }
                editor_cmdline[0] = '\0';
            } else {
                editor_cmdline[0] = '\0';
            }
            editor_display();
            return;
        } else if (key == 0x08) { 
            if (len > 0) editor_cmdline[len-1] = '\0';
            editor_display();
            return;
        } else if (len < 30 && key >= 32 && key <= 126) {
            editor_cmdline[len] = key;
            editor_cmdline[len+1] = '\0';
            editor_display();
            return;
        }
        return;
    }
    if (key == 0x1B) { 
        editor_mode = editor_mode == EDITOR_MODE_NORMAL ? EDITOR_MODE_INSERT : EDITOR_MODE_NORMAL;
        editor_display();
        return;
    }
    if (editor_mode == EDITOR_MODE_NORMAL && key == ':') {
        editor_cmdline_active = 1;
        editor_cmdline[0] = '\0';
        editor_display();
        return;
    }
    if (editor_mode == EDITOR_MODE_INSERT) {
        if (key == 0x08) { 
            if (editor_cursor > 0) {
                int len = strlen(editor_buffer);
                for (int i = editor_cursor - 1; i < len; i++) {
                    editor_buffer[i] = editor_buffer[i + 1];
                }
                editor_cursor--;
                if (editor_cursor < 0) editor_cursor = 0;
            }
            editor_display();
            return;
        }
        if (key == KEY_DEL_CODE) { 
            int len = strlen(editor_buffer);
            if (editor_cursor < len) {
                for (int i = editor_cursor; i < len; i++) {
                    editor_buffer[i] = editor_buffer[i + 1];
                }
            }
            editor_display();
            return;
        }
        if (key == '\n') {
            int len = strlen(editor_buffer);
            if (len < MAX_CONTENT - 1 && editor_cursor <= len) {
                for (int i = len; i >= editor_cursor; i--) {
                    editor_buffer[i + 1] = editor_buffer[i];
                }
                editor_buffer[editor_cursor] = '\n';
                editor_cursor++;
            }
            editor_display();
            return;
        }
        else if (key >= 32 && key <= 126) {
            int len = strlen(editor_buffer);
            if (len < MAX_CONTENT - 1 && editor_cursor <= len) {
                for (int i = len; i >= editor_cursor; i--) {
                    editor_buffer[i + 1] = editor_buffer[i];
                }
                editor_buffer[editor_cursor] = key;
                editor_cursor++;
            }
            editor_display();
            return;
        }
    }
    if (editor_cursor < 0) editor_cursor = 0;
    int buflen = strlen(editor_buffer);
    if (editor_cursor > buflen) editor_cursor = buflen;
    if (key == ARROW_LEFT) {
        if (editor_cursor > 0) editor_cursor--;
        editor_display();
        return;
    } else if (key == ARROW_RIGHT) {
        int len = strlen(editor_buffer);
        if (editor_cursor < len) editor_cursor++;
        editor_display();
        return;
    } else if (key == ARROW_UP) {
        if (editor_cursor > 0) {
            int col = 0;
            int cursor_temp = editor_cursor - 1;
            while (cursor_temp > 0 && editor_buffer[cursor_temp] != '\n') {
                cursor_temp--;
                col++;
            }
            if (cursor_temp > 0) { 
                cursor_temp--; 
                int line_start = cursor_temp;
                while (line_start > 0 && editor_buffer[line_start] != '\n') {
                    line_start--;
                }
                if (editor_buffer[line_start] == '\n') line_start++;
                int line_len = cursor_temp - line_start + 1;
                editor_cursor = line_start + (col < line_len ? col : line_len);
            }
        }
        editor_display();
        return;
    } else if (key == ARROW_DOWN) {
        int len = strlen(editor_buffer);
        if (editor_cursor < len) {
            int col = 0;
            int line_start = editor_cursor;
            while (line_start > 0 && editor_buffer[line_start-1] != '\n') {
                line_start--;
                col++;
            }
            int cursor_temp = editor_cursor;
            while (cursor_temp < len && editor_buffer[cursor_temp] != '\n') {
                cursor_temp++;
            }
            if (cursor_temp < len) { 
                cursor_temp++; 
                int next_line_start = cursor_temp;
                int line_len = 0;
                while (cursor_temp < len && editor_buffer[cursor_temp] != '\n') {
                    cursor_temp++;
                    line_len++;
                }
                editor_cursor = next_line_start + (col < line_len ? col : line_len);
            }
        }
        editor_display();
        return;
    } else if (key == KEY_HOME_CODE) {
        while (editor_cursor > 0 && editor_buffer[editor_cursor-1] != '\n') {
            editor_cursor--;
        }
        editor_display();
        return;
    } else if (key == KEY_END_CODE) {
        int len = strlen(editor_buffer);
        while (editor_cursor < len && editor_buffer[editor_cursor] != '\n') {
            editor_cursor++;
        }
        editor_display();
        return;
    } else if (key == KEY_PGUP_CODE) {
        for (int i = 0; i < 10; i++) {
            if (editor_cursor > 0) {
                editor_cursor--;
                while (editor_cursor > 0 && editor_buffer[editor_cursor] != '\n') {
                    editor_cursor--;
                }
            }
        }
        editor_display();
        return;
    } else if (key == KEY_PGDN_CODE) {
        int len = strlen(editor_buffer);
        for (int i = 0; i < 10; i++) {
            while (editor_cursor < len && editor_buffer[editor_cursor] != '\n') {
                editor_cursor++;
            }
            if (editor_cursor < len) editor_cursor++;
        }
        editor_display();
        return;
    }
    if (key == 0x01) { 
        while (editor_cursor > 0 && editor_buffer[editor_cursor-1] != '\n') {
            editor_cursor--;
        }
        editor_display();
        return;
    }
    if (key == 0x05) { 
        int len = strlen(editor_buffer);
        while (editor_cursor < len && editor_buffer[editor_cursor] != '\n') {
            editor_cursor++;
        }
        editor_display();
        return;
    }
    if (key == 0x04) { 
        int len = strlen(editor_buffer);
        if (editor_cursor < len) {
            for (int i = editor_cursor; editor_buffer[i] != '\0'; i++) {
                editor_buffer[i] = editor_buffer[i+1];
            }
        }
        editor_display();
        return;
    }
    editor_display();
}
void editor_run(char* filename) {
    int file_index = find_entry(filename);
    editor_state.should_exit = 0;
    if (file_index == -1) {
        file_index = create_file(filename, "");
        if (file_index < 0) {
            shell_print_string("Could not create file\n");
            return;
        }
    } else if (filesystem[file_index].type != TYPE_FILE) {
        shell_print_string("Not a file: ");
        shell_print_string(filename);
        shell_print_string("\n");
        return;
    } else if (!(filesystem[file_index].permissions & 2)) {
        shell_print_string("Permission denied: ");
        shell_print_string(filename);
        shell_print_string(" is not writable\n");
        return;
    }
    editor_init(file_index);
    editor_display(); 
    while (!editor_state.should_exit) {
        char key = get_char();
        if (key == 0) continue;
        editor_process_key(key);
    }
    clear_screen();
}
int find_matching_commands(const char* prefix, char matches[][128], int max_matches) {
    const char* commands[] = {
        "clear", "help", "help full", "shutdown", "color", "ls", "cd", "mkdir", 
        "pwd", "cat", "write", "rm", "chmod", "sysinfo", "vim", "fastfetch", "touch", "debug", "tree",
        "fat32", "fat32 init", "fat32 info", "fat32 switch",
        "help ls", "help cd", "help mkdir", "help touch", "help cat", "help vim", "help fat32",
        "help color", "help debug", "help tree", "help write", "help rm", "help chmod", "help pwd",
        "help sysinfo", "help fastfetch", "help shutdown", "help clear"
    };
    int num_commands = sizeof(commands) / sizeof(commands[0]);
    int count = 0;
    int prefix_len = 0;
    while (prefix[prefix_len] != '\0') prefix_len++;
    for (int i = 0; i < num_commands && count < max_matches; i++) {
        if (strncmp(prefix, commands[i], prefix_len) == 0) {
            int j = 0;
            while (commands[i][j] != '\0') {
                matches[count][j] = commands[i][j];
                j++;
            }
            matches[count][j] = '\0';
            count++;
        }
    }
    return count;
}
int find_matching_files(const char* prefix, char matches[][128], int max_matches, int only_files) {
    int count = 0;
    const char* lastslash = NULL;
    
    for (const char* p = prefix; *p; p++) {
        if (*p == '/') lastslash = p;
    }
    
    int search_dir;
    const char* fileprefix;
    char dirpath[256] = "";
    
    if (lastslash) {
        int len = lastslash - prefix;
        if (len > 0) {
            strncpy(dirpath, prefix, len);
            dirpath[len] = '\0';
            
            if (prefix[0] == '/') {
                search_dir = resolve_path_full(dirpath, 0);
            } else {
                char fullpath[512];
                get_current_path(fullpath);
                if (strlen(fullpath) > 1) strcat(fullpath, "/");
                strcat(fullpath, dirpath);
                search_dir = resolve_path_full(fullpath, 0);
                if (search_dir == -1) {
                    search_dir = resolve_path_full(dirpath, 0);
                }
            }
        } else {
            search_dir = 0;
        }
        fileprefix = lastslash + 1;
    } else {
        search_dir = current_dir;
        fileprefix = prefix;
    }
    
    if (search_dir == -1) return 0; 
    int fileprefix_len = strlen(fileprefix);
    for (int i = 0; i < fs_entry_count && count < max_matches; i++) {
        if (filesystem[i].parent_dir == search_dir) {
            if (strncmp(fileprefix, filesystem[i].name, fileprefix_len) == 0) {
                if (only_files && filesystem[i].type != TYPE_FILE) continue;
                int current_pos = 0;
                if (lastslash) {
                    int dirlen = strlen(dirpath);
                    if (dirlen > 0) {
                        strcpy(matches[count], dirpath);
                        current_pos += dirlen;
                        if (matches[count][current_pos-1] != '/') {
                            matches[count][current_pos++] = '/';
                        }
                    } else if (prefix[0] == '/') {
                        matches[count][current_pos++] = '/';
                    }
                }
                strcpy(matches[count] + current_pos, filesystem[i].name);
                if (filesystem[i].type == TYPE_DIR) {
                    strcat(matches[count], "/");
                }
                count++;
            }
        }
    }
    return count;
}
void print_entry_colored(FileEntry* entry) {
    if (entry->type == TYPE_DIR) {
        shell_print_colored(entry->name, COLOR_DIR, BLACK);
        shell_print_colored("/", COLOR_DIR, BLACK);
    } else if (entry->permissions & 4) {
        shell_print_colored(entry->name, COLOR_EXECUTABLE, BLACK);
        shell_print_colored("*", COLOR_EXECUTABLE, BLACK);
    } else if (!(entry->permissions & 2)) {
        shell_print_colored(entry->name, DARK_GRAY, BLACK);
    } else {
        shell_print_colored(entry->name, COLOR_FILE, BLACK);
    }
}
int resolve_path(const char* path) {
    int dir = current_dir;
    if (path[0] == '/') dir = 0; 
    char temp[256];
    strcpy(temp, path);
    char* saveptr;
    char* token = strtok_r(temp, "/", &saveptr);
    while (token) {
        if (strcmp(token, "..") == 0) {
            if (dir != 0) dir = filesystem[dir].parent_dir;
        } else if (strcmp(token, ".") == 0) {
        } else {
            int found = -1;
            for (int i = 0; i < fs_entry_count; i++) {
                if (i < fs_entry_count && filesystem[i].parent_dir == dir && strncmp(filesystem[i].name, token, MAX_FILENAME) == 0 && filesystem[i].type == TYPE_DIR) {
                    found = i;
                    break;
                }
            }
            if (found == -1) return -1;
            dir = found;
        }
        token = strtok_r(NULL, "/", &saveptr);
    }
    return dir;
}
int resolve_path_full(const char* path, int want_file) {
    if (!path || path[0] == '\0') {
        return want_file ? -1 : current_dir;
    }
    if (strcmp(path, "/") == 0) {
        return want_file ? -1 : 0; 
    }
    
    int dir = (path[0] == '/') ? 0 : current_dir;
    char temp[256];
    strcpy(temp, path);
    char* saveptr;
    char* token = strtok_r(temp, "/", &saveptr);
    
    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            if (dir != 0) {
                dir = filesystem[dir].parent_dir;
            }
        } else if (strcmp(token, ".") == 0) {
            
        } else {
            int found = -1;
            for (int i = 0; i < fs_entry_count; i++) {
                if (filesystem[i].parent_dir == dir && strcmp(filesystem[i].name, token) == 0) {
                    found = i;
                    break;
                }
            }
            
            if (found == -1) {
                return -1;
            }
            
            char* next_token = strtok_r(NULL, "/", &saveptr);
            if (next_token == NULL) {
                if (want_file) {
                    return found;
                } else {
                    return (filesystem[found].type == TYPE_DIR) ? found : -1;
                }
            } else {
                if (filesystem[found].type == TYPE_DIR) {
                    dir = found;
                } else {
                    return -1;
                }
                token = next_token;
                continue;
            }
        }
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    return want_file ? -1 : dir;
}
int mkdir_p(const char* path) {
    if (!path || path[0] == '\0') return -1;
    
    int dir = (path[0] == '/') ? 0 : current_dir;
    char temp[256];
    strcpy(temp, path);
    char* saveptr;
    char* token = strtok_r(temp, "/", &saveptr);
    
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            token = strtok_r(NULL, "/", &saveptr);
            continue;
        }
        if (strcmp(token, "..") == 0) {
            if (dir != 0) {
                dir = filesystem[dir].parent_dir;
            }
            token = strtok_r(NULL, "/", &saveptr);
            continue;
        }
        
        int found = -1;
        for (int i = 0; i < fs_entry_count; i++) {
            if (filesystem[i].parent_dir == dir && strcmp(filesystem[i].name, token) == 0) {
                if (filesystem[i].type == TYPE_DIR) {
                    found = i;
                    break;
                } else {
                    return -3;
                }
            }
        }
        
        if (found == -1) {
            if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
                return -1;
            }
            
            strcpy(filesystem[fs_entry_count].name, token);
            filesystem[fs_entry_count].content[0] = '\0';
            filesystem[fs_entry_count].type = TYPE_DIR;
            filesystem[fs_entry_count].permissions = 7;
            filesystem[fs_entry_count].parent_dir = dir;
            filesystem[fs_entry_count].size = 0;
            filesystem[fs_entry_count].created_time = fs_entry_count;
            
            dir = fs_entry_count;
            fs_entry_count++;
        } else {
            dir = found;
        }
        
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    return dir;
}
void print_with_pagination(const char* text) {
    int lines_printed = 0;
    int i = 0;
    int text_len = strlen(text);
    while (i < text_len) {
        while (i < text_len && text[i] != '\n') {
            shell_print_char(text[i]);
            i++;
        }
        if (i < text_len && text[i] == '\n') {
            shell_print_char('\n');
            i++;
            lines_printed++;
        }
        if (lines_printed >= 15) {
            set_color(YELLOW, BLACK);
            shell_print_string("-- Press SPACE for more, Q to quit, or any key to continue --");
            set_color(WHITE, BLACK);
            int key = 0;
            while ((key = get_char()) == 0); 
            if (key == 'q' || key == 'Q') {
                shell_print_char('\n');
                return; 
            } else if (key == ' ') {
                clear_screen(); 
                lines_printed = 0;
            } else {
                shell_print_char('\n');
                lines_printed = 0; 
            }
        }
    }
}
void show_quick_help() {
    shell_print_colored("=== SkyOS Quick Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_string("Basic Commands:\n");
    shell_print_string("  help              - Show this help\n");
    shell_print_string("  help full         - Show detailed help\n");
    shell_print_string("  help <command>    - Show help for specific command\n");
    shell_print_string("  clear             - Clear screen\n");
    shell_print_string("  ls [path]         - List files\n");
    shell_print_string("  cd <dir>          - Change directory\n");
    shell_print_string("  mkdir <dir>       - Create directory\n");
    shell_print_string("  touch <file>      - Create file\n");
    shell_print_string("  cat <file>        - Show file content\n");
    shell_print_string("  vim <file>        - Edit file\n");
    shell_print_string("  fat32 [cmd]       - FAT32 file system\n");
    shell_print_string("\nQuick Keys:\n");
    shell_print_string("  F1                - Quick help\n");
    shell_print_string("  F2                - Clear screen\n");
    shell_print_string("  Ctrl+C            - Cancel command\n");
    shell_print_string("  Tab               - Auto-complete\n");
    shell_print_string("\nExamples:\n");
    shell_print_string("  help ls           - Help for ls command\n");
    shell_print_string("  help fat32        - Help for fat32 command\n");
    shell_print_string("  help vim          - Help for vim editor\n");
    shell_print_string("\nTip: Type 'help full' for complete command list\n");
}
void show_full_help() {
    char help_text[4096];
    strcpy(help_text, 
        "=== SkyOS Complete Help ===\n\n"
        "SYSTEM COMMANDS:\n"
        "  clear                    - Clear the screen\n"
        "  help [full|command]     - Show help (full for detailed, command for specific)\n" 
        "  shutdown                - Power off the machine\n"
        "  color <fg> <bg>         - Change text color\n"
        "  sysinfo                 - Display system information\n"
        "  fastfetch               - System info with ASCII art\n\n"
        "FILE SYSTEM COMMANDS:\n"
        "  ls [path]               - List files in directory\n"
        "  cd <directory>          - Change to directory\n"
        "  mkdir <dirname>         - Create a directory\n"
        "  touch <filename>        - Create empty file\n"
        "  pwd                     - Print working directory\n"
        "  cat <filename>          - Show file content\n"
        "  write <file> \"text\"     - Write text to file\n"
        "  vim <filename>          - Edit file with editor\n"
        "  rm <filename>           - Remove a file\n"
        "  chmod <file> <perm>     - Change permissions (1=r,2=w,4=x)\n"
        "  tree                    - Show directory tree structure\n"
        "  debug <path>            - Debug path resolution issues\n\n"
        "FAT32 FILE SYSTEM:\n"
        "  fat32 init              - Initialize FAT32 file system (512KB)\n"
        "  fat32 info              - Show current file system information\n"
        "  fat32 switch            - Switch between RAM FS and FAT32\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "  F1                      - Quick help\n"
        "  F2                      - Clear screen\n"
        "  F3                      - Quick ls command\n"
        "  Ctrl+C                  - Cancel current command\n"
        "  Ctrl+L                  - Clear screen\n"
        "  Ctrl+D                  - Exit shell\n"
        "  Tab                     - Auto-completion\n"
        "  Arrow Keys              - Navigate cursor\n"
        "  Home/End                - Line beginning/end\n"
        "  Page Up/Down            - Page navigation\n\n"
        "EDITOR SHORTCUTS (vim):\n"
        "  ESC                     - Switch Normal/Insert mode\n"
        "  F1 (in editor)          - Editor help\n"
        "  F2 (in editor)          - Quick save\n"
        "  Ctrl+S                  - Save file\n"
        "  :w                      - Save file\n"
        "  :q                      - Quit editor\n"
        "  :x or :wq               - Save and quit\n\n"
        "EXAMPLES:\n"
        "  mkdir docs              - Create 'docs' directory\n"
        "  vim readme.txt          - Create/edit readme.txt\n"
        "  write test.txt \"hello\"  - Write 'hello' to test.txt\n"
        "  chmod test.txt 7        - Give all permissions\n"
        "  color 15 4             - White text on red background\n"
        "  help ls                 - Show help for ls command\n"
        "  help fat32              - Show help for fat32 command\n"
        "  help vim                - Show help for vim editor\n\n"
        "For more help, visit the SkyOS documentation.\n"
    );
    print_with_pagination(help_text);
}

void show_command_help(const char* command) {
    if (!command || command[0] == '\0') {
        show_quick_help();
        return;
    }
    
    if (strcmp(command, "ls") == 0) {
        show_ls_help();
    } else if (strcmp(command, "cd") == 0) {
        show_cd_help();
    } else if (strcmp(command, "mkdir") == 0) {
        show_mkdir_help();
    } else if (strcmp(command, "touch") == 0) {
        show_touch_help();
    } else if (strcmp(command, "cat") == 0) {
        show_cat_help();
    } else if (strcmp(command, "vim") == 0) {
        show_vim_help();
    } else if (strcmp(command, "fat32") == 0) {
        show_fat32_help();
    } else if (strcmp(command, "color") == 0) {
        show_color_help();
    } else if (strcmp(command, "debug") == 0) {
        show_debug_help();
    } else if (strcmp(command, "tree") == 0) {
        show_tree_help();
    } else if (strcmp(command, "write") == 0) {
        show_write_help();
    } else if (strcmp(command, "rm") == 0) {
        show_rm_help();
    } else if (strcmp(command, "chmod") == 0) {
        show_chmod_help();
    } else if (strcmp(command, "pwd") == 0) {
        show_pwd_help();
    } else if (strcmp(command, "sysinfo") == 0) {
        show_sysinfo_help();
    } else if (strcmp(command, "fastfetch") == 0) {
        show_fastfetch_help();
    } else if (strcmp(command, "shutdown") == 0) {
        show_shutdown_help();
    } else if (strcmp(command, "clear") == 0) {
        show_clear_help();
    } else {
        shell_print_colored("Help: ", COLOR_INFO, BLACK);
        shell_print_colored("Unknown command: ", COLOR_WARNING, BLACK);
        shell_print_colored(command, COLOR_ERROR, BLACK);
        shell_print_char('\n');
        shell_print_colored("Tip: ", COLOR_INFO, BLACK);
        shell_print_colored("Type 'help' to see all available commands\n", COLOR_INFO, BLACK);
    }
}

void print_tree(int dir_index, int depth) {
    for (int i = 0; i < depth; i++) {
        shell_print_string("  ");
    }
    
    if (dir_index == 0) {
        shell_print_colored("/", COLOR_DIR, BLACK);
    } else {
        shell_print_colored(filesystem[dir_index].name, COLOR_DIR, BLACK);
        shell_print_colored("/", COLOR_DIR, BLACK);
    }
    shell_print_char('\n');
    
    for (int i = 0; i < fs_entry_count; i++) {
        if (filesystem[i].parent_dir == dir_index) {
            for (int j = 0; j <= depth; j++) {
                shell_print_string("  ");
            }
            
            if (filesystem[i].type == TYPE_DIR) {
                shell_print_colored("├── ", COLOR_DIR, BLACK);
                shell_print_colored(filesystem[i].name, COLOR_DIR, BLACK);
                shell_print_colored("/", COLOR_DIR, BLACK);
                shell_print_char('\n');
                print_tree(i, depth + 1);
            } else {
                shell_print_colored("├── ", COLOR_FILE, BLACK);
                if (filesystem[i].permissions & 4) {
                    shell_print_colored(filesystem[i].name, COLOR_EXECUTABLE, BLACK);
                    shell_print_colored("*", COLOR_EXECUTABLE, BLACK);
                } else {
                    shell_print_colored(filesystem[i].name, COLOR_FILE, BLACK);
                }
                shell_print_char('\n');
            }
        }
    }
}

// ===== دوال المساعدة التفصيلية =====

void show_ls_help() {
    shell_print_colored("=== ls Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("ls [path]\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("List files and directories\n\n", WHITE, BLACK);
    shell_print_colored("Options:\n", COLOR_INFO, BLACK);
    shell_print_colored("  [path]  - Optional path to list (relative or absolute)\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  ls              - List current directory\n", WHITE, BLACK);
    shell_print_colored("  ls /home        - List /home directory\n", WHITE, BLACK);
    shell_print_colored("  ls docs         - List docs directory\n", WHITE, BLACK);
    shell_print_colored("  ls ../          - List parent directory\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Colored output (directories in blue, files in white)\n", WHITE, BLACK);
    shell_print_colored("  • Executable files marked with *\n", WHITE, BLACK);
    shell_print_colored("  • Works with both RAM FS and FAT32\n", WHITE, BLACK);
}

void show_cd_help() {
    shell_print_colored("=== cd Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("cd <directory>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Change current working directory\n\n", WHITE, BLACK);
    shell_print_colored("Arguments:\n", COLOR_INFO, BLACK);
    shell_print_colored("  <directory>  - Directory to change to\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  cd /home         - Change to /home\n", WHITE, BLACK);
    shell_print_colored("  cd docs          - Change to docs directory\n", WHITE, BLACK);
    shell_print_colored("  cd ..            - Go to parent directory\n", WHITE, BLACK);
    shell_print_colored("  cd /             - Go to root directory\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Supports relative and absolute paths\n", WHITE, BLACK);
    shell_print_colored("  • Shows current path in prompt\n", WHITE, BLACK);
    shell_print_colored("  • Error handling for non-existent directories\n", WHITE, BLACK);
}

void show_mkdir_help() {
    shell_print_colored("=== mkdir Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("mkdir <directory>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Create a new directory\n\n", WHITE, BLACK);
    shell_print_colored("Arguments:\n", COLOR_INFO, BLACK);
    shell_print_colored("  <directory>  - Name of directory to create\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  mkdir docs              - Create 'docs' directory\n", WHITE, BLACK);
    shell_print_colored("  mkdir projects/web      - Create nested directories\n", WHITE, BLACK);
    shell_print_colored("  mkdir /home/test        - Create with absolute path\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Creates parent directories automatically\n", WHITE, BLACK);
    shell_print_colored("  • Supports complex paths\n", WHITE, BLACK);
    shell_print_colored("  • Works with both RAM FS and FAT32\n", WHITE, BLACK);
    shell_print_colored("  • Colored success/error messages\n", WHITE, BLACK);
}

void show_touch_help() {
    shell_print_colored("=== touch Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("touch <filename>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Create an empty file\n\n", WHITE, BLACK);
    shell_print_colored("Arguments:\n", COLOR_INFO, BLACK);
    shell_print_colored("  <filename>  - Name of file to create\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  touch readme.txt        - Create readme.txt\n", WHITE, BLACK);
    shell_print_colored("  touch config.ini        - Create config.ini\n", WHITE, BLACK);
    shell_print_colored("  touch docs/file.txt     - Create in subdirectory\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Creates empty files instantly\n", WHITE, BLACK);
    shell_print_colored("  • Works with both RAM FS and FAT32\n", WHITE, BLACK);
    shell_print_colored("  • Supports relative and absolute paths\n", WHITE, BLACK);
    shell_print_colored("  • Warning if file already exists\n", WHITE, BLACK);
}

void show_cat_help() {
    shell_print_colored("=== cat Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("cat <filename>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Display file contents\n\n", WHITE, BLACK);
    shell_print_colored("Arguments:\n", COLOR_INFO, BLACK);
    shell_print_colored("  <filename>  - File to display\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  cat readme.txt          - Show readme.txt contents\n", WHITE, BLACK);
    shell_print_colored("  cat /home/config.ini    - Show with absolute path\n", WHITE, BLACK);
    shell_print_colored("  cat docs/file.txt       - Show file in subdirectory\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Displays file contents to screen\n", WHITE, BLACK);
    shell_print_colored("  • Supports complex paths\n", WHITE, BLACK);
    shell_print_colored("  • Error handling for non-existent files\n", WHITE, BLACK);
    shell_print_colored("  • Works with both RAM FS and FAT32\n", WHITE, BLACK);
}

void show_vim_help() {
    shell_print_colored("=== vim Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("vim <filename>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Edit file with text editor\n\n", WHITE, BLACK);
    shell_print_colored("Arguments:\n", COLOR_INFO, BLACK);
    shell_print_colored("  <filename>  - File to edit\n\n", WHITE, BLACK);
    shell_print_colored("Editor Modes:\n", COLOR_INFO, BLACK);
    shell_print_colored("  Normal Mode  - Navigation and commands\n", WHITE, BLACK);
    shell_print_colored("  Insert Mode  - Text editing\n\n", WHITE, BLACK);
    shell_print_colored("Keyboard Shortcuts:\n", COLOR_INFO, BLACK);
    shell_print_colored("  ESC          - Switch between modes\n", WHITE, BLACK);
    shell_print_colored("  F1           - Show editor help\n", WHITE, BLACK);
    shell_print_colored("  F2           - Quick save\n", WHITE, BLACK);
    shell_print_colored("  Ctrl+S       - Save file\n", WHITE, BLACK);
    shell_print_colored("  :w           - Save file\n", WHITE, BLACK);
    shell_print_colored("  :q           - Quit editor\n", WHITE, BLACK);
    shell_print_colored("  :x or :wq    - Save and quit\n", WHITE, BLACK);
}

void show_fat32_help() {
    shell_print_colored("=== fat32 Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("fat32 <subcommand>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Manage FAT32 file system\n\n", WHITE, BLACK);
    shell_print_colored("Subcommands:\n", COLOR_INFO, BLACK);
    shell_print_colored("  init    - Initialize FAT32 file system (512KB)\n", WHITE, BLACK);
    shell_print_colored("  info    - Show file system information\n", WHITE, BLACK);
    shell_print_colored("  switch  - Switch between RAM FS and FAT32\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  fat32 init              - Initialize FAT32\n", WHITE, BLACK);
    shell_print_colored("  fat32 info              - Show FS info\n", WHITE, BLACK);
    shell_print_colored("  fat32 switch            - Switch file systems\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • 512KB virtual disk\n", WHITE, BLACK);
    shell_print_colored("  • 4KB cluster size\n", WHITE, BLACK);
    shell_print_colored("  • Industry standard FAT32\n", WHITE, BLACK);
    shell_print_colored("  • Seamless switching between FS types\n", WHITE, BLACK);
}

void show_color_help() {
    shell_print_colored("=== color Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("color <foreground> <background>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Change text colors\n\n", WHITE, BLACK);
    shell_print_colored("Color Codes:\n", COLOR_INFO, BLACK);
    shell_print_colored("  0=Black, 1=Blue, 2=Green, 3=Cyan\n", WHITE, BLACK);
    shell_print_colored("  4=Red, 5=Magenta, 6=Brown, 7=Light Gray\n", WHITE, BLACK);
    shell_print_colored("  8=Dark Gray, 9=Light Blue, 10=Light Green\n", WHITE, BLACK);
    shell_print_colored("  11=Light Cyan, 12=Light Red, 13=Light Magenta\n", WHITE, BLACK);
    shell_print_colored("  14=Yellow, 15=White\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  color 15 0             - White on black\n", WHITE, BLACK);
    shell_print_colored("  color 10 1             - Light green on blue\n", WHITE, BLACK);
    shell_print_colored("  color 12 0             - Light red on black\n", WHITE, BLACK);
}

void show_debug_help() {
    shell_print_colored("=== debug Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("debug <path>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Debug path resolution issues\n\n", WHITE, BLACK);
    shell_print_colored("Arguments:\n", COLOR_INFO, BLACK);
    shell_print_colored("  <path>  - Path to debug\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  debug test/file.txt    - Debug specific path\n", WHITE, BLACK);
    shell_print_colored("  debug /home/docs       - Debug absolute path\n", WHITE, BLACK);
    shell_print_colored("  debug nonexistent      - Debug non-existent path\n\n", WHITE, BLACK);
    shell_print_colored("Output:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Current directory\n", WHITE, BLACK);
    shell_print_colored("  • Path resolution results\n", WHITE, BLACK);
    shell_print_colored("  • All filesystem entries\n", WHITE, BLACK);
    shell_print_colored("  • Parent directory relationships\n", WHITE, BLACK);
}

void show_tree_help() {
    shell_print_colored("=== tree Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("tree\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Display directory tree structure\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Shows complete directory hierarchy\n", WHITE, BLACK);
    shell_print_colored("  • Colored output (directories in blue)\n", WHITE, BLACK);
    shell_print_colored("  • Tree-like structure with ├── symbols\n", WHITE, BLACK);
    shell_print_colored("  • Executable files marked with *\n", WHITE, BLACK);
    shell_print_colored("  • Unlimited depth support\n", WHITE, BLACK);
    shell_print_colored("  • Works with RAM FS (FAT32 support coming)\n", WHITE, BLACK);
}

void show_write_help() {
    shell_print_colored("=== write Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("write <file> \"<text>\"\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Write text to file\n\n", WHITE, BLACK);
    shell_print_colored("Arguments:\n", COLOR_INFO, BLACK);
    shell_print_colored("  <file>  - File to write to\n", WHITE, BLACK);
    shell_print_colored("  <text>  - Text to write (in quotes)\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  write test.txt \"Hello World\"\n", WHITE, BLACK);
    shell_print_colored("  write config.ini \"[settings]\\nport=8080\"\n", WHITE, BLACK);
    shell_print_colored("  write docs/readme.txt \"Project documentation\"\n", WHITE, BLACK);
}

void show_rm_help() {
    shell_print_colored("=== rm Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("rm <filename>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Remove (delete) a file\n\n", WHITE, BLACK);
    shell_print_colored("Arguments:\n", COLOR_INFO, BLACK);
    shell_print_colored("  <filename>  - File to delete\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  rm test.txt             - Delete test.txt\n", WHITE, BLACK);
    shell_print_colored("  rm /home/file.txt       - Delete with absolute path\n", WHITE, BLACK);
    shell_print_colored("  rm docs/old.txt         - Delete in subdirectory\n", WHITE, BLACK);
}

void show_chmod_help() {
    shell_print_colored("=== chmod Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("chmod <file> <permissions>\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Change file permissions\n\n", WHITE, BLACK);
    shell_print_colored("Permission Codes:\n", COLOR_INFO, BLACK);
    shell_print_colored("  1 = Read permission\n", WHITE, BLACK);
    shell_print_colored("  2 = Write permission\n", WHITE, BLACK);
    shell_print_colored("  4 = Execute permission\n", WHITE, BLACK);
    shell_print_colored("  7 = All permissions (1+2+4)\n\n", WHITE, BLACK);
    shell_print_colored("Examples:\n", COLOR_INFO, BLACK);
    shell_print_colored("  chmod test.txt 7        - Give all permissions\n", WHITE, BLACK);
    shell_print_colored("  chmod script.sh 5       - Read + Execute\n", WHITE, BLACK);
    shell_print_colored("  chmod config.ini 3      - Read + Write\n", WHITE, BLACK);
}

void show_pwd_help() {
    shell_print_colored("=== pwd Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("pwd\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Print working directory\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Shows current directory path\n", WHITE, BLACK);
    shell_print_colored("  • Always starts with /\n", WHITE, BLACK);
    shell_print_colored("  • Updates when using cd command\n", WHITE, BLACK);
}

void show_sysinfo_help() {
    shell_print_colored("=== sysinfo Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("sysinfo\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Display system information\n\n", WHITE, BLACK);
    shell_print_colored("Information Shown:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Operating system name and version\n", WHITE, BLACK);
    shell_print_colored("  • System architecture\n", WHITE, BLACK);
    shell_print_colored("  • Memory information\n", WHITE, BLACK);
    shell_print_colored("  • File system statistics\n", WHITE, BLACK);
}

void show_fastfetch_help() {
    shell_print_colored("=== fastfetch Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("fastfetch\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Display system information with ASCII art\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • ASCII art logo\n", WHITE, BLACK);
    shell_print_colored("  • Colored system information\n", WHITE, BLACK);
    shell_print_colored("  • Aesthetic display\n", WHITE, BLACK);
}

void show_shutdown_help() {
    shell_print_colored("=== shutdown Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("shutdown\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Power off the system\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Graceful system shutdown\n", WHITE, BLACK);
    shell_print_colored("  • Saves any pending data\n", WHITE, BLACK);
    shell_print_colored("  • Exits QEMU emulator\n", WHITE, BLACK);
}

void show_clear_help() {
    shell_print_colored("=== clear Command Help ===\n", LIGHT_GREEN, BLACK);
    shell_print_colored("Usage: ", COLOR_INFO, BLACK);
    shell_print_colored("clear\n\n", COLOR_WARNING, BLACK);
    shell_print_colored("Description: ", COLOR_INFO, BLACK);
    shell_print_colored("Clear the screen\n\n", WHITE, BLACK);
    shell_print_colored("Features:\n", COLOR_INFO, BLACK);
    shell_print_colored("  • Clears entire screen\n", WHITE, BLACK);
    shell_print_colored("  • Resets cursor position\n", WHITE, BLACK);
    shell_print_colored("  • Also available via F2 key\n", WHITE, BLACK);
}
