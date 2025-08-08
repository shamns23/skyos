#ifndef FILESYSTEM_H
#define FILESYSTEM_H

// Filesystem constants
#define MAX_FILES 20
#define MAX_FILENAME 32
#define MAX_CONTENT 512
#define MAX_DIRS 10
#define MAX_DIRNAME 32
#define TYPE_FILE 1
#define TYPE_DIR 2

// File entry structure
typedef struct {
    char name[MAX_FILENAME];
    char content[MAX_CONTENT];
    unsigned char type;
    unsigned short permissions;
    int parent_dir;
    int size;
    int created_time;
} FileEntry;

// Global filesystem variables
extern FileEntry filesystem[MAX_FILES + MAX_DIRS];
extern int fs_entry_count;
extern int current_dir;
extern int use_fat32;
extern unsigned int fat32_current_cluster;

// Function declarations
void init_filesystem();
void get_current_path(char* buffer);
int find_entry(const char* name);
int create_file(const char* name, const char* content);
int create_directory(const char* name);
void print_entry_colored(FileEntry* entry);
int resolve_path(const char* path);
int resolve_path_full(const char* path, int want_file);
int mkdir_p(const char* path);

#endif // FILESYSTEM_H