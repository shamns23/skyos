#include <stddef.h>
#include "fat32.h"
#include "string_utils.h"
#include "display.h"
#include "keyboard.h"
#include "filesystem.h"
#include "memory.h"

#include "shell.h"
#include "command_handler.h"
#include "fastfetch.h"
#include "hardware_detection.h"
// Global variables for kernel
// Display variables moved to display.c
// Editor variables moved to editor.c
#define PIT_FREQ 100
volatile unsigned int system_ticks = 0;
extern void irq0_handler();

// Timer interrupt handler
void timer_handler() {
    system_ticks++;
}
void shell_print_string(const char* str);
// Help function declarations moved to shell.h
// Display and I/O functions are now in separate modules
// Keyboard functions moved to keyboard.c
// Filesystem structures and functions moved to filesystem.c
// Filesystem functions moved to filesystem.c
// delay function moved to io.c
// shutdown function moved to shell.c
void process_cmd(char* cmd);
void readline(char* buffer, int max_len);
int find_matching_commands(const char* prefix, char matches[][128], int max_matches);
int find_matching_files(const char* prefix, char matches[][128], int max_matches, int only_files);
// Editor function declarations moved to editor.h
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
    shell_print_string("Welcome to oszoOS! [v4.1 - Advanced Operating System]\n");
    set_color(WHITE, BLACK);
    shell_print_string("Type 'help' for a list of commands.\n");
    shell_print_string("Enhanced keyboard & FAT32 support. Use 'fat32 init' to initialize FAT32 FS.\n\n");
    init_keyboard();   
    init_filesystem();
    memory_init();
    fastfetch_init();
    
    shell_print_colored("Initializing FAT32 file system...\n", COLOR_INFO, BLACK);
    if (fat32_init() == 0) {
        shell_print_colored("FAT32 initialized successfully (512KB)\n", COLOR_SUCCESS, BLACK);
    } else {
        shell_print_colored("FAT32 initialization failed\n", COLOR_ERROR, BLACK);
    }
    
    // Initialize hardware detection and display system information using fastfetch
    shell_print_colored("\nDetecting hardware...\n", COLOR_INFO, BLACK);
    hardware_detection_init();
    shell_print_colored("System Information:\n", COLOR_INFO, BLACK);
    display_fastfetch_style(); 
    char cmd_buffer[128];
    char current_path[256];
    while (1) {
        get_current_path(current_path);
        shell_print_colored("oszoOS", COLOR_SUCCESS, BLACK);
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
    // Use the new improved command handler
    int handled = process_command(cmd);
    
    // If command was handled by new system, return
    if (handled) {
        return;
    }
    
    // Keep legacy commands that haven't been migrated yet
    char* saveptr;
    char* command = strtok_r(cmd, " ", &saveptr);
    if (command == NULL) { 
        return;
    }
    
    // Check if command was already handled by new system
    if (strcmp(command, "clear") == 0 ||
        strcmp(command, "help") == 0 ||
        strcmp(command, "ls") == 0 ||
        strcmp(command, "cd") == 0 ||
        strcmp(command, "pwd") == 0 ||
        strcmp(command, "mkdir") == 0 ||
        strcmp(command, "cat") == 0 ||
        strcmp(command, "rm") == 0 ||
        strcmp(command, "chmod") == 0 ||
        strcmp(command, "touch") == 0 ||
        strcmp(command, "color") == 0 ||

        strcmp(command, "fastfetch") == 0 ||
        strcmp(command, "write") == 0 ||
        strcmp(command, "run") == 0 ||
        strcmp(command, "shutdown") == 0 ||
        strcmp(command, "fat32") == 0 ||
        strcmp(command, "debug") == 0) {
        return; // Already handled by new system
    }
    
    // Legacy commands (to be migrated)
    if (strcmp(command, "write") == 0) {
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
                    shell_print_colored("oszoOS", COLOR_SUCCESS, BLACK);
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
// Editor variables moved to editor.c
// editor_display function moved to editor.c
// Editor functions moved to editor.c
int find_matching_commands(const char* prefix, char matches[][128], int max_matches) {
    const char* commands[] = {
        "clear", "help", "help full", "shutdown", "color", "ls", "cd", "mkdir", 
        "pwd", "cat", "write", "rm", "chmod", "fastfetch", "touch", "debug",
        "fat32", "fat32 init", "fat32 info", "fat32 switch",
        "help ls", "help cd", "help mkdir", "help touch", "help cat", "help fat32",
        "help color", "help debug", "help write", "help rm", "help chmod", "help pwd",
        "help sysinfo", "help fastfetch", "help shutdown", "help clear"
    };
    int num_commands = sizeof(commands) / sizeof(commands[0]);
    int count = 0;
    int prefix_len = 0;
    while (prefix[prefix_len] != '\0') prefix_len++;
    for (int i = 0; i < num_commands && count < max_matches; i++) {
        if (my_strncmp(prefix, commands[i], prefix_len) == 0) {
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
            if (my_strncmp(fileprefix, filesystem[i].name, fileprefix_len) == 0) {
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
