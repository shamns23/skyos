#include "command_handler.h"
#include <string.h> // For strrchr, strtok_r, etc.
#include "display.h"
#include "filesystem.h"
#include "shell.h"
#include "string_utils.h"
#include "fat32.h"
#include "memory.h"

// Command handler functions
static void cmd_clear(char* args __attribute__((unused))) {
    clear_screen();
}

static void cmd_help(char* args) {
    char* saveptr;
    char* option = strtok_r(args, " ", &saveptr);
    
    if (option) {
        // Check for --full option
        if (my_strncmp(option, "--full", 6) == 0) {
            show_full_help();
        } else {
            // Show help for specific command
            show_command_help(option);
        }
    } else {
        // Show quick help by default
        show_quick_help();
    }
}

static void cmd_ls(char* args) {
    char* saveptr;
    char* path = strtok_r(args, " ", &saveptr);
    
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
}

static void cmd_cd(char* args) {
    if (args) {
        // Skip leading spaces
        while (*args == ' ') args++;
    }
    
    char* saveptr;
    char* dirname = strtok_r(args, " ", &saveptr);
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
}

static void cmd_pwd(char* args __attribute__((unused))) {
    char path[256];
    get_current_path(path);
    shell_print_string(path);
    shell_print_string("\n");
}

static void cmd_mkdir(char* args) {
    if (!args) {
        shell_print_colored("Usage: ", COLOR_INFO, BLACK);
        shell_print_colored("mkdir <dirname>\n", COLOR_WARNING, BLACK);
        return;
    }
    
    // Skip leading spaces
    while (*args == ' ') args++;
    
    if (*args == '\0') {
        shell_print_colored("Usage: ", COLOR_INFO, BLACK);
        shell_print_colored("mkdir <dirname>\n", COLOR_WARNING, BLACK);
        return;
    }
    
    // Use the entire remaining string as dirname
    char* dirname = args;
    
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
}

static void cmd_cat(char* args) {
    if (!args) {
        shell_print_string("Usage: cat <filename>\n");
        return;
    }
    
    // Skip leading spaces
    while (*args == ' ') args++;
    
    if (*args == '\0') {
        shell_print_string("Usage: cat <filename>\n");
        return;
    }
    
    // Use the entire remaining string as filename (handle spaces in names)
    char* filename = args;
    
    int file_index = resolve_path_full(filename, 1);
    if (file_index == -1) {
        shell_print_colored("Error: ", COLOR_ERROR, BLACK);
        shell_print_colored("File not found: ", COLOR_ERROR, BLACK);
        shell_print_colored(filename, COLOR_WARNING, BLACK);
        shell_print_colored("\n", COLOR_ERROR, BLACK);
    } else if (filesystem[file_index].type != TYPE_FILE) {
        shell_print_colored("Error: ", COLOR_ERROR, BLACK);
        shell_print_colored("Not a file: ", COLOR_ERROR, BLACK);
        shell_print_colored(filename, COLOR_WARNING, BLACK);
        shell_print_char('\n');
    } else if (!(filesystem[file_index].permissions & 1)) {
        shell_print_colored("Error: ", COLOR_ERROR, BLACK);
        shell_print_string("Permission denied: ");
        shell_print_colored(filename, COLOR_WARNING, BLACK);
        shell_print_string(" is not readable\n");
    } else {
        shell_print_string(filesystem[file_index].content);
        shell_print_string("\n");
    }
}

static void cmd_rm(char* args) {
    if (!args) {
        shell_print_string("Usage: rm <filename>\n");
        return;
    }
    
    // Skip leading spaces
    while (*args == ' ') args++;
    
    if (*args == '\0') {
        shell_print_string("Usage: rm <filename>\n");
        return;
    }
    
    // Use the entire remaining string as filename
    char* filename = args;
    
    int file_index = resolve_path_full(filename, 1);
    if (file_index == -1) {
        shell_print_colored("Error: ", COLOR_ERROR, BLACK);
        shell_print_colored("File not found: ", COLOR_ERROR, BLACK);
        shell_print_colored(filename, COLOR_WARNING, BLACK);
        shell_print_char('\n');
    } else if (filesystem[file_index].type == TYPE_DIR) {
        shell_print_colored("Error: ", COLOR_ERROR, BLACK);
        shell_print_string("Cannot remove directory: ");
        shell_print_colored(filename, COLOR_WARNING, BLACK);
        shell_print_char('\n');
    } else {
        filesystem[file_index] = filesystem[fs_entry_count - 1];
        fs_entry_count--;
        shell_print_colored("Success: ", COLOR_SUCCESS, BLACK);
        shell_print_colored("File removed: ", COLOR_SUCCESS, BLACK);
        shell_print_colored(filename, COLOR_FILE, BLACK);
        shell_print_char('\n');
    }
}

static void cmd_chmod(char* args) {
    if (!args) {
        shell_print_string("Usage: chmod <filename> <mode>\n");
        shell_print_string("Permission modes (octal):\n");
        shell_print_string("644 - Read/write for owner, read for others\n");
        shell_print_string("755 - Read/write/execute for owner, read/execute for others\n");
        shell_print_string("777 - Full permissions for everyone\n");
        return;
    }
    
    // Skip leading spaces
    while (*args == ' ') args++;
    
    if (*args == '\0') {
        shell_print_string("Usage: chmod <filename> <mode>\n");
        return;
    }
    
    // Find the last space to separate filename from permission
    char* last_space = NULL;
    for (char* p = args; *p; p++) {
        if (*p == ' ') {
            last_space = p;
        }
    }
    if (!last_space) {
        shell_print_string("Usage: chmod <filename> <mode>\n");
        return;
    }
    
    // Split filename and permission
    *last_space = '\0';
    char* filename = args;
    char* perm_str = last_space + 1;
    
    // Trim trailing spaces in filename
    char* end = filename + my_strlen(filename) - 1;
    while (end > filename && *end == ' ') {
        *end = '\0';
        end--;
    }
    
    // Skip leading spaces in perm_str
    while (*perm_str == ' ') perm_str++;
    
    if (*perm_str == '\0' || *filename == '\0') {
        shell_print_string("Usage: chmod <filename> <mode>\n");
        return;
    }
    
    // Parse permission value (3-digit octal)
    int perm = 0;
    
    // Extract only octal digits
    char digits[4] = {0};
    int digit_count = 0;
    
    for (char* p = perm_str; *p && digit_count < 3; p++) {
        if (*p >= '0' && *p <= '7') {
            digits[digit_count++] = *p;
        }
    }
    
    if (digit_count != 3) {
        shell_print_string("Invalid permission value. Use 3-digit octal (000-777)\n");
        return;
    }
    
    // Convert digits to integer
    perm = (digits[0] - '0') * 100 + (digits[1] - '0') * 10 + (digits[2] - '0');
    
    if (perm < 0 || perm > 777) {
        shell_print_string("Invalid permission value. Use 3-digit octal (000-777)\n");
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
        shell_print_colored(filename, COLOR_WARNING, BLACK);
        shell_print_char('\n');
    } else {
        filesystem[file_index].permissions = perm;
        shell_print_colored("Success: ", COLOR_SUCCESS, BLACK);
        shell_print_colored("Permissions changed for ", COLOR_SUCCESS, BLACK);
        shell_print_colored(filename, COLOR_FILE, BLACK);
        shell_print_colored(" to ", COLOR_SUCCESS, BLACK);
        char perm_str_display[4];
        perm_str_display[0] = (perm / 100) + '0';
        perm_str_display[1] = ((perm / 10) % 10) + '0';
        perm_str_display[2] = (perm % 10) + '0';
        perm_str_display[3] = '\0';
        shell_print_colored(perm_str_display, COLOR_WARNING, BLACK);
        shell_print_char('\n');
    }
}

static void cmd_color(char* args) {
    char* saveptr;
    char* fg_str = strtok_r(args, " ", &saveptr);
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
}

#include "sysinfo.h"

static void cmd_sysinfo(char* args __attribute__((unused))) {
    display_detailed_sysinfo();
}

static void cmd_memory(char* args) {
    if (!args) {
        shell_print_colored("Memory Information:\n", COLOR_INFO, BLACK);
        
        char buffer[32];
        
        shell_print_colored("Total: ", LIGHT_GREEN, BLACK);
        itoa(memory_get_total() / 1024, buffer);
        shell_print_string(buffer);
        shell_print_string(" KB\n");
        
        shell_print_colored("Used: ", LIGHT_GREEN, BLACK);
        itoa(memory_get_used() / 1024, buffer);
        shell_print_string(buffer);
        shell_print_string(" KB\n");
        
        shell_print_colored("Free: ", LIGHT_GREEN, BLACK);
        itoa(memory_get_free() / 1024, buffer);
        shell_print_string(buffer);
        shell_print_string(" KB\n");
        
        shell_print_colored("Usage: ", LIGHT_GREEN, BLACK);
        int usage = (memory_get_used() * 100) / memory_get_total();
        itoa(usage, buffer);
        shell_print_string(buffer);
        shell_print_string("%\n");
        
        return;
    }
    
    char* saveptr;
    char* subcommand = strtok_r(args, " ", &saveptr);
    
    if (my_strncmp(subcommand, "dump", 4) == 0) {
        memory_dump();
    } else if (my_strncmp(subcommand, "check", 5) == 0) {
        if (memory_check_integrity()) {
            shell_print_colored("Memory integrity: OK\n", COLOR_SUCCESS, BLACK);
        } else {
            shell_print_colored("Memory integrity: FAILED\n", COLOR_ERROR, BLACK);
        }
    } else if (my_strncmp(subcommand, "test", 4) == 0) {
        shell_print_colored("Testing memory allocation...\n", COLOR_INFO, BLACK);
        
        void* ptr1 = malloc(100);
        void* ptr2 = malloc(200);
        void* ptr3 = malloc(50);
        
        if (ptr1 && ptr2 && ptr3) {
            shell_print_colored("✓ Allocations successful\n", COLOR_SUCCESS, BLACK);
            
            free(ptr2);
            shell_print_colored("✓ Free test successful\n", COLOR_SUCCESS, BLACK);
            
            void* ptr4 = malloc(150);
            if (ptr4) {
                shell_print_colored("✓ Reallocation test successful\n", COLOR_SUCCESS, BLACK);
                free(ptr4);
            }
            
            free(ptr1);
            free(ptr3);
        } else {
            shell_print_colored("✗ Memory allocation failed\n", COLOR_ERROR, BLACK);
        }
    } else {
        shell_print_colored("Usage: memory <dump|check|test>\n", COLOR_INFO, BLACK);
    }
}

static void cmd_fastfetch(char* args __attribute__((unused))) {
    display_fastfetch_style();
}

static void cmd_hardware(char* args __attribute__((unused))) {
    display_detailed_sysinfo();
}

static void cmd_write(char* args) {
    char* saveptr;
    char* filename = strtok_r(args, " ", &saveptr);
    char* content_start = strchr(saveptr, '"');
    if (filename && content_start) {
        content_start++; 
        char* content_end = strchr(content_start, '"');
        if (content_end) {
            *content_end = '\0'; 
            int file_index = resolve_path_full(filename, 1);
            if (file_index != -1) {
                if (filesystem[file_index].type != TYPE_FILE) {
                    shell_print_string("Error: Not a file\n");
                    return;
                }
                my_strncpy(filesystem[file_index].content, content_start, MAX_CONTENT - 1);
                filesystem[file_index].content[MAX_CONTENT - 1] = '\0';
                shell_print_string("File updated: ");
                shell_print_string(filename);
                shell_print_string("\n");
            } else {
                int result = create_file(filename, content_start);
                if (result == -1) {
                    shell_print_string("Filesystem is full!\n");
                } else {
                    shell_print_string("File created: ");
                    shell_print_string(filename);
                    shell_print_string("\n");
                }
            }
        } else {
            shell_print_string("Write format error: missing closing quote.\n");
        }
    } else {
        shell_print_string("Usage: write <filename> \"content\"\n");
    }
}

static void cmd_run(char* args) {
    char* saveptr;
    char* filename = strtok_r(args, " ", &saveptr);
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
}

static void cmd_shutdown(char* args __attribute__((unused))) {
    shell_print_colored("Shutting down system...\n", COLOR_WARNING, BLACK);
    shutdown();
}

static void cmd_fat32(char* args) {
    char* saveptr;
    char* subcommand = strtok_r(args, " ", &saveptr);
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
}

static void cmd_touch(char* args) {
    if (!args) {
        shell_print_string("Usage: touch <filename>\n");
        return;
    }
    
    // Skip leading spaces
    while (*args == ' ') args++;
    
    if (*args == '\0') {
        shell_print_string("Usage: touch <filename>\n");
        return;
    }
    
    // Use the entire remaining string as filename
    char* filename = args;
    
    int file_index = resolve_path_full(filename, 1);
    if (file_index != -1) {
        // File exists, update timestamp (simplified)
        shell_print_string("File already exists: ");
        shell_print_string(filename);
        shell_print_string("\n");
        return;
    }
    
    // Create empty file
    int result = create_file(filename, "");
    if (result == -1) {
        shell_print_string("Error: Cannot create file\n");
    } else {
        shell_print_string("File created: ");
        shell_print_string(filename);
        shell_print_string("\n");
    }
}

static void cmd_debug(char* args) {
    char* saveptr;
    char* path = strtok_r(args, " ", &saveptr);
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
}

// Command table
static const CommandEntry command_table[] = {
    {"clear", cmd_clear},
    {"help", cmd_help},
    {"ls", cmd_ls},
    {"cd", cmd_cd},
    {"pwd", cmd_pwd},
    {"mkdir", cmd_mkdir},
    {"cat", cmd_cat},
    {"rm", cmd_rm},
    {"chmod", cmd_chmod},
    {"touch", cmd_touch},
    {"color", cmd_color},
    {"sysinfo", cmd_sysinfo},
    {"fastfetch", cmd_fastfetch},
    {"hardware", cmd_hardware},
    {"write", cmd_write},
    {"run", cmd_run},
    {"shutdown", cmd_shutdown},
    {"fat32", cmd_fat32},
    {"debug", cmd_debug},
    {"memory", cmd_memory},
    {NULL, NULL} // End marker
};

// Main command processor
int process_command(char* cmd) {
    if (!cmd || *cmd == '\0') {
        return 0;
    }
    
    char* saveptr;
    char* command = strtok_r(cmd, " ", &saveptr);
    char* args = saveptr; // Remaining arguments
    
    // Skip leading spaces in args
    if (args) {
        while (*args == ' ') args++;
        if (*args == '\0') args = NULL; // If only spaces, set to NULL
    }
    
    if (!command) {
        return 0;
    }
    
    // Search for command in table - use exact match instead of partial match
    for (int i = 0; command_table[i].name != NULL; i++) {
        if (strcmp(command, command_table[i].name) == 0) {
            command_table[i].handler(args);
            return 1; // Command found and executed
        }
    }
    
    // Command not found
    return 0;
}