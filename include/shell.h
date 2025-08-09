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
void show_sysinfo_help();
void show_fastfetch_help();
void show_shutdown_help();
void show_clear_help();

#endif // SHELL_H