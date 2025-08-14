#ifndef SHELL_H
#define SHELL_H

// Function declarations for shell commands and help system
void shutdown();
void print_with_pagination(const char* text);
void show_quick_help();
void show_full_help();
void show_command_help(const char* command);


// Individual help functions
void show_ls_help();
void show_cd_help();
void show_mkdir_help();
void show_touch_help();
void show_cat_help();

void show_fat32_help();
void show_color_help();
void show_debug_help();

void show_write_help();
void show_rm_help();
void show_chmod_help();
void show_pwd_help();

void show_fastfetch_help();
void show_hardware_help();
void show_shutdown_help();
void show_clear_help();
void show_memory_help();

void readline(char* buffer, int max_len);
int find_matching_commands(const char* prefix, char matches[][128], int max_matches);
int find_matching_files(const char* prefix, char matches[][128], int max_matches, int only_files);
void get_current_path(char* buffer);

#endif // SHELL_H