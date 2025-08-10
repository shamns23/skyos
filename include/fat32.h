#ifndef FAT32_H
#define FAT32_H

// FAT32 File System Header
// oszoOS v4.1 - Advanced File System

// FAT32 Directory Entry Structure
typedef struct {
    char name[11];
    unsigned char attributes;
    unsigned char reserved;
    unsigned char creation_time_tenth;
    unsigned short creation_time;
    unsigned short creation_date;
    unsigned short last_access_date;
    unsigned short cluster_high;
    unsigned short last_write_time;
    unsigned short last_write_date;
    unsigned short cluster_low;
    unsigned int file_size;
} __attribute__((packed)) FAT32_DirEntry;

// Forward declarations
typedef struct FAT32_FileSystem FAT32_FileSystem;

// FAT32 Function Prototypes
int fat32_init();
int fat32_format(unsigned int total_size_kb);

// File operations
FAT32_DirEntry* fat32_find_file(const char* path);
int fat32_create_file(const char* filename, unsigned int parent_cluster);
int fat32_create_directory(const char* dirname, unsigned int parent_cluster);

// Directory operations
void fat32_list_directory(unsigned int cluster);
unsigned int fat32_get_root_cluster();

// Cluster management
unsigned int fat32_get_cluster_value(unsigned int cluster);
void fat32_set_cluster_value(unsigned int cluster, unsigned int value);
unsigned int fat32_allocate_cluster();
void fat32_free_cluster_chain(unsigned int first_cluster);
unsigned char* fat32_get_cluster_data(unsigned int cluster);

// Utility functions
char fat32_name_to_8_3(const char* filename, char* name_8_3);
FAT32_DirEntry* fat32_find_file_in_cluster(unsigned int cluster, const char* filename);

// FAT32 constants
#define FAT32_MAX_FILENAME 255
#define FAT32_SECTOR_SIZE 512
#define FAT32_CLUSTER_SIZE 4096

#endif // FAT32_H