#include "command_handler.h"
#include "display.h"
#include "filesystem.h"
#include "shell.h"
#include "string_utils.h"
#include "fat32.h"
#include "editor.h"

// Command handler functions
static void cmd_clear(char* args) {
    clear_screen();
}

static void cmd_help(char* args) {
    char* saveptr;
    char* option = strtok_r(args, " ", &saveptr);
    if (option && my_strncmp(option, "full", 4) == 0) {
        show_full_help();
    } else if (option) {
        show_command_help(option);
    } else {
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

static void cmd_pwd(char* args) {
    char path[256];
    get_current_path(path);
    shell_print_string(path);
    shell_print_string("\n");
}

static void cmd_mkdir(char* args) {
    char* saveptr;
    char* dirname = strtok_r(args, " ", &saveptr);
    
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
}

static void cmd_cat(char* args) {
    char* saveptr;
    char* filename = strtok_r(args, " ", &saveptr);
    
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
}

static void cmd_rm(char* args) {
    char* saveptr;
    char* filename = strtok_r(args, " ", &saveptr);
    
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
}

static void cmd_chmod(char* args) {
    char* saveptr;
    char* filename = strtok_r(args, " ", &saveptr);
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
    {NULL, NULL} // End marker
};

// Main command processor
void process_command(char* cmd) {
    if (!cmd || *cmd == '\0') {
        return;
    }
    
    char* saveptr;
    char* command = strtok_r(cmd, " ", &saveptr);
    char* args = saveptr; // Remaining arguments
    
    if (!command) {
        return;
    }
    
    // Search for command in table
    for (int i = 0; command_table[i].name != NULL; i++) {
        if (my_strncmp(command, command_table[i].name, my_strlen(command_table[i].name)) == 0) {
            command_table[i].handler(args);
            return;
        }
    }
    
    // Command not found
    shell_print_colored("Unknown command: ", COLOR_ERROR, BLACK);
    shell_print_colored(command, COLOR_WARNING, BLACK);
    shell_print_string("\nType 'help' for available commands.\n");
}