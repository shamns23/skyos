#include "fat32.h"
#include <stddef.h>

// FAT32 Boot Sector Structure
typedef struct {
    unsigned char jump_code[3];
    char oem_name[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char fat_count;
    unsigned short root_entries;
    unsigned short total_sectors_16;
    unsigned char media_descriptor;
    unsigned short fat_size_16;
    unsigned short sectors_per_track;
    unsigned short heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_32;
    unsigned int fat_size_32;
    unsigned short flags;
    unsigned short version;
    unsigned int root_cluster;
    unsigned short fs_info_sector;
    unsigned short backup_boot_sector;
    unsigned char reserved[12];
    unsigned char drive_number;
    unsigned char reserved_1;
    unsigned char signature;
    unsigned int volume_serial;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed)) FAT32_BootSector;

// FAT32 Long File Name Entry
typedef struct {
    unsigned char order;
    unsigned short name1[5];
    unsigned char attributes;
    unsigned char type;
    unsigned char checksum;
    unsigned short name2[6];
    unsigned short cluster;
    unsigned short name3[2];
} __attribute__((packed)) FAT32_LFNEntry;

// FAT32 File System Structure
typedef struct FAT32_FileSystem {
    FAT32_BootSector boot_sector;
    unsigned int *fat_table;
    unsigned char *data_area;
    unsigned int fat_start_sector;
    unsigned int data_start_sector;
    unsigned int total_clusters;
    unsigned int current_cluster;
    unsigned int root_dir_cluster;
} FAT32_FileSystem;

// Global FAT32 instance
static FAT32_FileSystem fat32_fs;
static unsigned char disk_buffer[512 * 1024]; // 512KB disk simulation

// FAT32 Constants
#define FAT32_CLUSTER_FREE     0x00000000
#define FAT32_CLUSTER_EOC      0x0FFFFFF8
#define FAT32_CLUSTER_BAD      0x0FFFFFF7
#define FAT32_CLUSTER_RESERVED 0x0FFFFFF0

#define FAT32_ATTR_READ_ONLY   0x01
#define FAT32_ATTR_HIDDEN      0x02
#define FAT32_ATTR_SYSTEM      0x04
#define FAT32_ATTR_VOLUME_ID   0x08
#define FAT32_ATTR_DIRECTORY   0x10
#define FAT32_ATTR_ARCHIVE     0x20
#define FAT32_ATTR_LFN         0x0F

// Helper Functions
unsigned int fat32_get_cluster_value(unsigned int cluster) {
    if (cluster >= fat32_fs.total_clusters) return FAT32_CLUSTER_EOC;
    return fat32_fs.fat_table[cluster] & 0x0FFFFFFF;
}

void fat32_set_cluster_value(unsigned int cluster, unsigned int value) {
    if (cluster >= fat32_fs.total_clusters) return;
    fat32_fs.fat_table[cluster] = value & 0x0FFFFFFF;
}

unsigned int fat32_allocate_cluster() {
    for (unsigned int i = 2; i < fat32_fs.total_clusters; i++) {
        if (fat32_get_cluster_value(i) == FAT32_CLUSTER_FREE) {
            fat32_set_cluster_value(i, FAT32_CLUSTER_EOC);
            return i;
        }
    }
    return 0; // No free clusters
}

void fat32_free_cluster_chain(unsigned int first_cluster) {
    unsigned int cluster = first_cluster;
    while (cluster != FAT32_CLUSTER_EOC && cluster != FAT32_CLUSTER_FREE) {
        unsigned int next_cluster = fat32_get_cluster_value(cluster);
        fat32_set_cluster_value(cluster, FAT32_CLUSTER_FREE);
        cluster = next_cluster;
    }
}

unsigned char* fat32_get_cluster_data(unsigned int cluster) {
    if (cluster < 2 || cluster >= fat32_fs.total_clusters) return NULL;
    
    unsigned int cluster_size = fat32_fs.boot_sector.bytes_per_sector * 
                               fat32_fs.boot_sector.sectors_per_cluster;
    unsigned int data_offset = (cluster - 2) * cluster_size;
    
    return fat32_fs.data_area + data_offset;
}

// FAT32 Initialization
int fat32_format(unsigned int total_size_kb) {
    // Clear disk buffer
    for (int i = 0; i < sizeof(disk_buffer); i++) {
        disk_buffer[i] = 0;
    }
    
    // Setup boot sector
    FAT32_BootSector *boot = (FAT32_BootSector*)disk_buffer;
    
    // Jump instruction
    boot->jump_code[0] = 0xEB;
    boot->jump_code[1] = 0x58;
    boot->jump_code[2] = 0x90;
    
    // OEM name
    for (int i = 0; i < 8; i++) boot->oem_name[i] = ' ';
    boot->oem_name[0] = 'S'; boot->oem_name[1] = 'h'; boot->oem_name[2] = 'a';
    boot->oem_name[3] = 'd'; boot->oem_name[4] = 'o'; boot->oem_name[5] = 'w';
    boot->oem_name[6] = 'O'; boot->oem_name[7] = 'S';
    
    // Basic parameters
    boot->bytes_per_sector = 512;
    boot->sectors_per_cluster = 8; // 4KB clusters
    boot->reserved_sectors = 32;
    boot->fat_count = 2;
    boot->root_entries = 0; // FAT32 doesn't use this
    boot->total_sectors_16 = 0; // Use 32-bit field
    boot->media_descriptor = 0xF8;
    boot->fat_size_16 = 0; // Use 32-bit field
    boot->sectors_per_track = 63;
    boot->heads = 16;
    boot->hidden_sectors = 0;
    
    // Calculate sizes
    unsigned int total_sectors = (total_size_kb * 1024) / 512;
    boot->total_sectors_32 = total_sectors;
    
    unsigned int fat_size = (total_sectors - boot->reserved_sectors) / 
                           (boot->sectors_per_cluster * 128 + boot->fat_count);
    boot->fat_size_32 = fat_size;
    
    // FAT32 specific
    boot->flags = 0;
    boot->version = 0;
    boot->root_cluster = 2;
    boot->fs_info_sector = 1;
    boot->backup_boot_sector = 6;
    
    for (int i = 0; i < 12; i++) boot->reserved[i] = 0;
    
    boot->drive_number = 0x80;
    boot->reserved_1 = 0;
    boot->signature = 0x29;
    boot->volume_serial = 0x12345678;
    
    // Volume label
    for (int i = 0; i < 11; i++) boot->volume_label[i] = ' ';
    boot->volume_label[0] = 'S'; boot->volume_label[1] = 'H'; 
    boot->volume_label[2] = 'A'; boot->volume_label[3] = 'D';
    boot->volume_label[4] = 'O'; boot->volume_label[5] = 'W';
    boot->volume_label[6] = 'O'; boot->volume_label[7] = 'S';
    
    // FS Type
    for (int i = 0; i < 8; i++) boot->fs_type[i] = ' ';
    boot->fs_type[0] = 'F'; boot->fs_type[1] = 'A';
    boot->fs_type[2] = 'T'; boot->fs_type[3] = '3';
    boot->fs_type[4] = '2';
    
    // Boot signature
    disk_buffer[510] = 0x55;
    disk_buffer[511] = 0xAA;
    
    // Setup file system structure
    fat32_fs.boot_sector = *boot;
    fat32_fs.fat_start_sector = boot->reserved_sectors;
    fat32_fs.data_start_sector = boot->reserved_sectors + (boot->fat_count * boot->fat_size_32);
    fat32_fs.total_clusters = (total_sectors - fat32_fs.data_start_sector) / boot->sectors_per_cluster;
    fat32_fs.root_dir_cluster = boot->root_cluster;
    
    // Allocate FAT table
    fat32_fs.fat_table = (unsigned int*)(disk_buffer + fat32_fs.fat_start_sector * 512);
    fat32_fs.data_area = disk_buffer + fat32_fs.data_start_sector * 512;
    
    // Initialize FAT table
    fat32_fs.fat_table[0] = 0x0FFFFF00 | boot->media_descriptor;
    fat32_fs.fat_table[1] = 0x0FFFFFFF;
    fat32_fs.fat_table[2] = FAT32_CLUSTER_EOC; // Root directory
    
    for (unsigned int i = 3; i < fat32_fs.total_clusters; i++) {
        fat32_fs.fat_table[i] = FAT32_CLUSTER_FREE;
    }
    
    // Create root directory
    unsigned char *root_data = fat32_get_cluster_data(2);
    FAT32_DirEntry *root_entry = (FAT32_DirEntry*)root_data;
    
    // Create volume label entry
    for (int i = 0; i < 11; i++) root_entry->name[i] = boot->volume_label[i];
    root_entry->attributes = FAT32_ATTR_VOLUME_ID;
    root_entry->reserved = 0;
    root_entry->creation_time_tenth = 0;
    root_entry->creation_time = 0;
    root_entry->creation_date = 0;
    root_entry->last_access_date = 0;
    root_entry->cluster_high = 0;
    root_entry->last_write_time = 0;
    root_entry->last_write_date = 0;
    root_entry->cluster_low = 0;
    root_entry->file_size = 0;
    
    return 0;
}

// Directory operations
char fat32_name_to_8_3(const char* filename, char* name_8_3) {
    int name_len = 0, ext_len = 0;
    const char* dot_pos = NULL;
    
    // Find extension
    for (int i = 0; filename[i]; i++) {
        if (filename[i] == '.') dot_pos = &filename[i];
    }
    
    // Clear name buffer
    for (int i = 0; i < 11; i++) name_8_3[i] = ' ';
    
    // Copy name part
    for (int i = 0; i < 8 && filename[i] && filename[i] != '.'; i++) {
        if (filename[i] >= 'a' && filename[i] <= 'z') {
            name_8_3[i] = filename[i] - 'a' + 'A'; // Convert to uppercase
        } else {
            name_8_3[i] = filename[i];
        }
        name_len++;
    }
    
    // Copy extension
    if (dot_pos) {
        for (int i = 0; i < 3 && dot_pos[i+1]; i++) {
            if (dot_pos[i+1] >= 'a' && dot_pos[i+1] <= 'z') {
                name_8_3[8+i] = dot_pos[i+1] - 'a' + 'A';
            } else {
                name_8_3[8+i] = dot_pos[i+1];
            }
            ext_len++;
        }
    }
    
    return (name_len <= 8 && ext_len <= 3) ? 1 : 0; // Return 1 if valid 8.3 name
}

FAT32_DirEntry* fat32_find_file_in_cluster(unsigned int cluster, const char* filename) {
    unsigned char *cluster_data = fat32_get_cluster_data(cluster);
    if (!cluster_data) return NULL;
    
    char name_8_3[11];
    fat32_name_to_8_3(filename, name_8_3);
    
    unsigned int cluster_size = fat32_fs.boot_sector.bytes_per_sector * 
                               fat32_fs.boot_sector.sectors_per_cluster;
    unsigned int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);
    
    FAT32_DirEntry *entries = (FAT32_DirEntry*)cluster_data;
    
    for (unsigned int i = 0; i < entries_per_cluster; i++) {
        if (entries[i].name[0] == 0) break; // End of directory
        if ((unsigned char)entries[i].name[0] == 0xE5) continue; // Deleted entry
        if (entries[i].attributes == FAT32_ATTR_LFN) continue; // LFN entry
        
        // Compare names
        int match = 1;
        for (int j = 0; j < 11; j++) {
            if (entries[i].name[j] != name_8_3[j]) {
                match = 0;
                break;
            }
        }
        
        if (match) return &entries[i];
    }
    
    return NULL;
}

FAT32_DirEntry* fat32_find_file(const char* path) {
    if (!path || path[0] == '\0') return NULL;
    
    unsigned int current_cluster = fat32_fs.root_dir_cluster;
    char temp_path[256];
    
    // Copy path to temporary buffer
    int path_len = 0;
    while (path[path_len] && path_len < 255) {
        temp_path[path_len] = path[path_len];
        path_len++;
    }
    temp_path[path_len] = '\0';
    
    // Handle absolute path
    if (temp_path[0] == '/') {
        current_cluster = fat32_fs.root_dir_cluster;
        // Skip leading slash
        for (int i = 0; i < path_len; i++) {
            temp_path[i] = temp_path[i+1];
        }
        path_len--;
    }
    
    // Parse path components
    char *token = temp_path;
    char *next_slash;
    
    while (token && *token) {
        // Find next slash
        next_slash = NULL;
        for (int i = 0; token[i]; i++) {
            if (token[i] == '/') {
                next_slash = &token[i];
                break;
            }
        }
        
        // Null-terminate current component
        if (next_slash) *next_slash = '\0';
        
        // Find file/directory in current cluster
        FAT32_DirEntry *entry = fat32_find_file_in_cluster(current_cluster, token);
        if (!entry) return NULL;
        
        // Get cluster number
        current_cluster = ((unsigned int)entry->cluster_high << 16) | entry->cluster_low;
        
        // Move to next component
        if (next_slash) {
            token = next_slash + 1;
        } else {
            // This is the last component
            return entry;
        }
        
        // Check if this is a directory
        if (!(entry->attributes & FAT32_ATTR_DIRECTORY)) {
            return NULL; // Not a directory but path continues
        }
    }
    
    return NULL;
}

int fat32_create_file(const char* filename, unsigned int parent_cluster) {
    if (!filename) return -1;
    
    // Find free directory entry
    unsigned char *cluster_data = fat32_get_cluster_data(parent_cluster);
    if (!cluster_data) return -1;
    
    unsigned int cluster_size = fat32_fs.boot_sector.bytes_per_sector * 
                               fat32_fs.boot_sector.sectors_per_cluster;
    unsigned int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);
    
    FAT32_DirEntry *entries = (FAT32_DirEntry*)cluster_data;
    FAT32_DirEntry *free_entry = NULL;
    
    for (unsigned int i = 0; i < entries_per_cluster; i++) {
        if (entries[i].name[0] == 0 || (unsigned char)entries[i].name[0] == 0xE5) {
            free_entry = &entries[i];
            break;
        }
    }
    
    if (!free_entry) return -1; // No free entry
    
    // Create directory entry
    char name_8_3[11];
    if (!fat32_name_to_8_3(filename, name_8_3)) return -1;
    
    for (int i = 0; i < 11; i++) free_entry->name[i] = name_8_3[i];
    free_entry->attributes = FAT32_ATTR_ARCHIVE;
    free_entry->reserved = 0;
    free_entry->creation_time_tenth = 0;
    free_entry->creation_time = 0;
    free_entry->creation_date = 0;
    free_entry->last_access_date = 0;
    free_entry->cluster_high = 0;
    free_entry->last_write_time = 0;
    free_entry->last_write_date = 0;
    free_entry->cluster_low = 0;
    free_entry->file_size = 0;
    
    return 0;
}

int fat32_create_directory(const char* dirname, unsigned int parent_cluster) {
    if (!dirname) return -1;
    
    // Allocate cluster for new directory
    unsigned int new_cluster = fat32_allocate_cluster();
    if (new_cluster == 0) return -1;
    
    // Create directory entry in parent
    unsigned char *parent_data = fat32_get_cluster_data(parent_cluster);
    if (!parent_data) {
        fat32_set_cluster_value(new_cluster, FAT32_CLUSTER_FREE);
        return -1;
    }
    
    unsigned int cluster_size = fat32_fs.boot_sector.bytes_per_sector * 
                               fat32_fs.boot_sector.sectors_per_cluster;
    unsigned int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);
    
    FAT32_DirEntry *entries = (FAT32_DirEntry*)parent_data;
    FAT32_DirEntry *free_entry = NULL;
    
    for (unsigned int i = 0; i < entries_per_cluster; i++) {
        if (entries[i].name[0] == 0 || (unsigned char)entries[i].name[0] == 0xE5) {
            free_entry = &entries[i];
            break;
        }
    }
    
    if (!free_entry) {
        fat32_set_cluster_value(new_cluster, FAT32_CLUSTER_FREE);
        return -1;
    }
    
    // Create directory entry
    char name_8_3[11];
    if (!fat32_name_to_8_3(dirname, name_8_3)) {
        fat32_set_cluster_value(new_cluster, FAT32_CLUSTER_FREE);
        return -1;
    }
    
    for (int i = 0; i < 11; i++) free_entry->name[i] = name_8_3[i];
    free_entry->attributes = FAT32_ATTR_DIRECTORY;
    free_entry->reserved = 0;
    free_entry->creation_time_tenth = 0;
    free_entry->creation_time = 0;
    free_entry->creation_date = 0;
    free_entry->last_access_date = 0;
    free_entry->cluster_high = (new_cluster >> 16) & 0xFFFF;
    free_entry->last_write_time = 0;
    free_entry->last_write_date = 0;
    free_entry->cluster_low = new_cluster & 0xFFFF;
    free_entry->file_size = 0;
    
    // Initialize new directory cluster
    unsigned char *new_dir_data = fat32_get_cluster_data(new_cluster);
    FAT32_DirEntry *new_entries = (FAT32_DirEntry*)new_dir_data;
    
    // Clear directory
    for (int i = 0; i < cluster_size; i++) {
        new_dir_data[i] = 0;
    }
    
    // Create "." entry
    for (int i = 0; i < 11; i++) new_entries[0].name[i] = ' ';
    new_entries[0].name[0] = '.';
    new_entries[0].attributes = FAT32_ATTR_DIRECTORY;
    new_entries[0].cluster_high = (new_cluster >> 16) & 0xFFFF;
    new_entries[0].cluster_low = new_cluster & 0xFFFF;
    
    // Create ".." entry
    for (int i = 0; i < 11; i++) new_entries[1].name[i] = ' ';
    new_entries[1].name[0] = '.';
    new_entries[1].name[1] = '.';
    new_entries[1].attributes = FAT32_ATTR_DIRECTORY;
    new_entries[1].cluster_high = (parent_cluster >> 16) & 0xFFFF;
    new_entries[1].cluster_low = parent_cluster & 0xFFFF;
    
    return 0;
}

// Integration functions for existing OS
int fat32_init() {
    return fat32_format(512); // 512KB disk
}

void fat32_list_directory(unsigned int cluster) {
    unsigned char *cluster_data = fat32_get_cluster_data(cluster);
    if (!cluster_data) return;
    
    unsigned int cluster_size = fat32_fs.boot_sector.bytes_per_sector * 
                               fat32_fs.boot_sector.sectors_per_cluster;
    unsigned int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);
    
    FAT32_DirEntry *entries = (FAT32_DirEntry*)cluster_data;
    
    for (unsigned int i = 0; i < entries_per_cluster; i++) {
        if (entries[i].name[0] == 0) break; // End of directory
        if ((unsigned char)entries[i].name[0] == 0xE5) continue; // Deleted entry
        if (entries[i].attributes == FAT32_ATTR_LFN) continue; // LFN entry
        if (entries[i].attributes == FAT32_ATTR_VOLUME_ID) continue; // Volume label
        
        // Print filename
        for (int j = 0; j < 8; j++) {
            if (entries[i].name[j] != ' ') {
                // Use shell_print_char from kernel
                extern void shell_print_char(char c);
                shell_print_char(entries[i].name[j]);
            }
        }
        
        // Print extension if exists
        if (entries[i].name[8] != ' ') {
            extern void shell_print_char(char c);
            shell_print_char('.');
            for (int j = 8; j < 11; j++) {
                if (entries[i].name[j] != ' ') {
                    shell_print_char(entries[i].name[j]);
                }
            }
        }
        
        // Print directory indicator
        if (entries[i].attributes & FAT32_ATTR_DIRECTORY) {
            extern void shell_print_string(const char* str);
            shell_print_string("/");
        }
        
        extern void shell_print_string(const char* str);
        shell_print_string("  ");
    }
}

unsigned int fat32_get_root_cluster() {
    return fat32_fs.root_dir_cluster;
}