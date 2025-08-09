#include <stddef.h>
#include "fat32.h"
#include "string_utils.h"
#include "display.h"
#include "keyboard.h"
#include "filesystem.h"
#include "editor.h"
#include "shell.h"
#include "command_handler.h"
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
    if (my_strncmp(command, "clear", 5) == 0 ||
        my_strncmp(command, "help", 4) == 0 ||
        my_strncmp(command, "ls", 2) == 0 ||
        my_strncmp(command, "cd", 2) == 0 ||
        my_strncmp(command, "pwd", 3) == 0 ||
        my_strncmp(command, "mkdir", 5) == 0 ||
        my_strncmp(command, "cat", 3) == 0 ||
        my_strncmp(command, "rm", 2) == 0 ||
        my_strncmp(command, "chmod", 5) == 0) {
        return; // Already handled by new system
    }
    
    // Legacy commands (to be migrated)
    if (my_strncmp(command, "cat", 3) == 0) {
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
    } else if (my_strncmp(command, "touch", 5) == 0) {
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
    } else if (my_strncmp(command, "pwd", 3) == 0) {
        char path[256];
        get_current_path(path);
        shell_print_string(path);
        shell_print_string("\n");
    } else if (my_strncmp(command, "cat", 3) == 0) {
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
    } else if (my_strncmp(command, "write", 5) == 0) {
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
    } else if (my_strncmp(command, "rm", 2) == 0) {
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
    } else if (my_strncmp(command, "chmod", 5) == 0) {
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
    } else if (my_strncmp(command, "color", 5) == 0) {
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
    } else if (my_strncmp(command, "sysinfo", 7) == 0) {
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
    } else if (my_strncmp(command, "fastfetch", 9) == 0) {
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
    } else if (my_strncmp(command, "vim", 3) == 0) {
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
    } else if (my_strncmp(command, "run", 3) == 0) {
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
    } else if (my_strncmp(command, "touch", 5) == 0) {
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
    } else if (my_strncmp(command, "shutdown", 8) == 0) {
        shell_print_colored("Shutting down system...\n", COLOR_WARNING, BLACK);
        shutdown();
    } else if (my_strncmp(command, "tree", 4) == 0) {
        shell_print_colored("Directory tree:\n", COLOR_INFO, BLACK);
        if (use_fat32) {
            shell_print_colored("FAT32 tree view not implemented yet\n", COLOR_WARNING, BLACK);
        } else {
            print_tree(0, 0);
        }
        return;
    } else if (my_strncmp(command, "fat32", 5) == 0) {
        char* subcommand = strtok_r(NULL, " ", &saveptr);
        if (!subcommand) {
            shell_print_colored("Usage: fat32 <init|info|switch>\n", COLOR_INFO, BLACK);
            return;
        }
        
        if (my_strncmp(subcommand, "init", 4) == 0) {
            shell_print_colored("Initializing FAT32 file system...\n", COLOR_INFO, BLACK);
            if (fat32_init() == 0) {
                shell_print_colored("Success: ", COLOR_SUCCESS, BLACK);
                shell_print_colored("FAT32 file system initialized (512KB)\n", COLOR_SUCCESS, BLACK);
            } else {
                shell_print_colored("Error: ", COLOR_ERROR, BLACK);
                shell_print_colored("Failed to initialize FAT32\n", COLOR_ERROR, BLACK);
            }
        } else if (my_strncmp(subcommand, "info", 4) == 0) {
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
        } else if (my_strncmp(subcommand, "switch", 6) == 0) {
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
    } else if (my_strncmp(command, "debug", 5) == 0) {
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
// Editor variables moved to editor.c
// editor_display function moved to editor.c
// Editor functions moved to editor.c
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
