#include "filesystem.h"
#include "string_utils.h"
#include "display.h"

// Global filesystem variables
FileEntry filesystem[MAX_FILES + MAX_DIRS];
int fs_entry_count = 0;
int current_dir = 0;
int use_fat32 = 0;
unsigned int fat32_current_cluster = 0;

void init_filesystem() {
    // Initialize root directory
    strcpy(filesystem[0].name, "/");
    filesystem[0].type = TYPE_DIR;
    filesystem[0].permissions = 0755;
    filesystem[0].parent_dir = -1;
    filesystem[0].size = 0;
    filesystem[0].created_time = 0;
    fs_entry_count = 1;
    current_dir = 0;
    
    // Create some default directories
    create_directory("home");
    create_directory("bin");
    create_directory("etc");
    create_directory("tmp");
    
    // Create some default files
    create_file("readme.txt", "Welcome to SkyOS!\nThis is a simple operating system.\n");
    create_file("version.txt", "SkyOS v1.0\nBuilt with love\n");
}

void get_current_path(char* buffer) {
    if (current_dir == 0) {
        strcpy(buffer, "/");
        return;
    }
    
    char temp_path[256];
    temp_path[0] = '\0';
    
    int dir = current_dir;
    while (dir != -1 && dir != 0) {
        char temp[256];
        strcpy(temp, "/");
        strcat(temp, filesystem[dir].name);
        strcat(temp, temp_path);
        strcpy(temp_path, temp);
        dir = filesystem[dir].parent_dir;
    }
    
    strcpy(buffer, temp_path);
    if (buffer[0] == '\0') strcpy(buffer, "/");
}

int find_entry(const char* name) {
    for (int i = 0; i < fs_entry_count; i++) {
        if (filesystem[i].parent_dir == current_dir && strcmp(filesystem[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int create_file(const char* name, const char* content) {
    if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
        shell_print_colored("Error: Maximum file limit reached\n", COLOR_ERROR, BLACK);
        return -1;
    }
    
    if (find_entry(name) != -1) {
        shell_print_colored("Error: File already exists\n", COLOR_ERROR, BLACK);
        return -1;
    }
    
    strcpy(filesystem[fs_entry_count].name, name);
    if (content) {
        my_strncpy(filesystem[fs_entry_count].content, content, MAX_CONTENT - 1);
        filesystem[fs_entry_count].content[MAX_CONTENT - 1] = '\0';
    } else {
        filesystem[fs_entry_count].content[0] = '\0';
    }
    filesystem[fs_entry_count].type = TYPE_FILE;
    filesystem[fs_entry_count].permissions = 0644;
    filesystem[fs_entry_count].parent_dir = current_dir;
    filesystem[fs_entry_count].size = content ? my_strlen(content) : 0;
    filesystem[fs_entry_count].created_time = 0;
    
    return fs_entry_count++;
}

int create_directory(const char* name) {
    if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
        shell_print_colored("Error: Maximum directory limit reached\n", COLOR_ERROR, BLACK);
        return -1;
    }
    
    if (find_entry(name) != -1) {
        shell_print_colored("Error: Directory already exists\n", COLOR_ERROR, BLACK);
        return -1;
    }
    
    strcpy(filesystem[fs_entry_count].name, name);
    filesystem[fs_entry_count].type = TYPE_DIR;
    filesystem[fs_entry_count].permissions = 0755;
    filesystem[fs_entry_count].parent_dir = current_dir;
    filesystem[fs_entry_count].size = 0;
    filesystem[fs_entry_count].created_time = 0;
    
    return fs_entry_count++;
}

void print_entry_colored(FileEntry* entry) {
    if (entry->type == TYPE_DIR) {
        shell_print_colored(entry->name, COLOR_DIR, BLACK);
        shell_print_colored("/", COLOR_DIR, BLACK);
    } else {
        if (entry->permissions & 0111) {
            shell_print_colored(entry->name, COLOR_EXECUTABLE, BLACK);
        } else {
            shell_print_colored(entry->name, COLOR_FILE, BLACK);
        }
    }
}

int resolve_path(const char* path) {
    return resolve_path_full(path, 0);
}

int resolve_path_full(const char* path, int want_file) {
    if (!path || path[0] == '\0') return current_dir;
    
    int dir = current_dir;
    if (path[0] == '/') {
        dir = 0;
        path++;
    }
    
    if (path[0] == '\0') return dir;
    
    char path_copy[256];
    my_strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    
    char* saveptr;
    char* token = strtok_r(path_copy, "/", &saveptr);
    
    while (token) {
        if (strcmp(token, ".") == 0) {
            // Current directory, do nothing
        } else if (strcmp(token, "..") == 0) {
            // Parent directory
            if (dir != 0) {
                dir = filesystem[dir].parent_dir;
                if (dir == -1) dir = 0;
            }
        } else {
            // Find the entry in current directory
            int found = -1;
            for (int i = 0; i < fs_entry_count; i++) {
                if (filesystem[i].parent_dir == dir && strcmp(filesystem[i].name, token) == 0) {
                    found = i;
                    break;
                }
            }
            
            if (found == -1) {
                return -1; // Path not found
            }
            
            char* next_token = strtok_r(NULL, "/", &saveptr);
            if (next_token == NULL) {
                // This is the last component
                if (want_file && filesystem[found].type != TYPE_FILE) {
                    return -1; // Want file but found directory
                }
                return found;
            } else {
                // More components to process
                if (filesystem[found].type != TYPE_DIR) {
                    return -1; // Not a directory
                }
                dir = found;
            }
        }
        
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    return dir;
}

int mkdir_p(const char* path) {
    if (!path || path[0] == '\0') return -1;
    
    char path_copy[256];
    my_strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    
    int original_dir = current_dir;
    
    if (path_copy[0] == '/') {
        current_dir = 0;
    }
    
    char* saveptr;
    char* token = strtok_r(path_copy, "/", &saveptr);
    
    while (token) {
        if (strcmp(token, ".") == 0 || strcmp(token, "..") == 0) {
            token = strtok_r(NULL, "/", &saveptr);
            continue;
        }
        
        int found = find_entry(token);
        if (found == -1) {
            // Directory doesn't exist, create it
            int new_dir = create_directory(token);
            if (new_dir == -1) {
                current_dir = original_dir;
                return -1;
            }
            current_dir = new_dir;
        } else {
            if (filesystem[found].type != TYPE_DIR) {
                current_dir = original_dir;
                return -1; // Path component is not a directory
            }
            current_dir = found;
        }
        
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    current_dir = original_dir;
    return 0;
}