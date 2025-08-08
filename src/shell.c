#include "shell.h"
#include "display.h"
#include "keyboard.h"
#include "filesystem.h"
#include "string_utils.h"
#include "io.h"

void shutdown() {
    shell_print_colored("Shutting down SkyOS...\n", COLOR_INFO, BLACK);
    delay();
    
    // ACPI shutdown
    outw(0x604, 0x2000);
    
    // APM shutdown
    outw(0xB004, 0x2000);
    
    // VirtualBox shutdown
    outw(0x4004, 0x3400);
    
    // QEMU shutdown
    outb(0x501, 0x01);
    
    shell_print_colored("System halted.\n", COLOR_WARNING, BLACK);
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void print_with_pagination(const char* text) {
    int lines = 0;
    const char* ptr = text;
    
    while (*ptr) {
        shell_print_char(*ptr);
        if (*ptr == '\n') {
            lines++;
            if (lines >= 20) {
                shell_print_colored("\n--- Press any key to continue ---", COLOR_PROMPT, BLACK);
                get_char();
                shell_print_string("\n");
                lines = 0;
            }
        }
        ptr++;
    }
}

void show_quick_help() {
    shell_print_colored("\n=== SkyOS Quick Help ===\n", COLOR_INFO, BLACK);
    shell_print_string("Basic Commands:\n");
    shell_print_colored("  ls", COLOR_EXECUTABLE, BLACK);
    shell_print_string("        - List files and directories\n");
    shell_print_colored("  cd <dir>", COLOR_EXECUTABLE, BLACK);
    shell_print_string("   - Change directory\n");
    shell_print_colored("  mkdir <dir>", COLOR_EXECUTABLE, BLACK);
    shell_print_string(" - Create directory\n");
    shell_print_colored("  touch <file>", COLOR_EXECUTABLE, BLACK);
    shell_print_string(" - Create file\n");
    shell_print_colored("  cat <file>", COLOR_EXECUTABLE, BLACK);
    shell_print_string("  - Display file content\n");
    shell_print_colored("  vim <file>", COLOR_EXECUTABLE, BLACK);
    shell_print_string("  - Edit file\n");
    shell_print_colored("  clear", COLOR_EXECUTABLE, BLACK);
    shell_print_string("     - Clear screen\n");
    shell_print_colored("  help", COLOR_EXECUTABLE, BLACK);
    shell_print_string("      - Show this help\n");
    shell_print_colored("  shutdown", COLOR_EXECUTABLE, BLACK);
    shell_print_string("  - Shutdown system\n");
    shell_print_string("\nType 'help <command>' for detailed help on a specific command.\n");
    shell_print_string("Type 'help --full' for complete documentation.\n\n");
}

void show_full_help() {
    const char* help_text = 
        "\n=== SkyOS Complete Documentation ===\n\n"
        "BASIC COMMANDS:\n"
        "  ls [options]     - List directory contents\n"
        "    -l             - Long format with details\n"
        "    -a             - Show hidden files\n\n"
        "  cd <directory>   - Change current directory\n"
        "    cd ..          - Go to parent directory\n"
        "    cd /           - Go to root directory\n"
        "    cd ~           - Go to home directory\n\n"
        "  pwd              - Print working directory\n\n"
        "FILE OPERATIONS:\n"
        "  touch <filename> - Create empty file\n"
        "  cat <filename>   - Display file contents\n"
        "  vim <filename>   - Edit file with text editor\n"
        "  rm <filename>    - Remove file\n"
        "  write <file> <content> - Write content to file\n\n"
        "DIRECTORY OPERATIONS:\n"
        "  mkdir <dirname>  - Create directory\n"
        "  mkdir -p <path>  - Create directory path\n"
        "  tree             - Show directory tree\n\n"
        "SYSTEM COMMANDS:\n"
        "  clear            - Clear screen\n"
        "  sysinfo          - Show system information\n"
        "  fastfetch        - Show system info with style\n"
        "  shutdown         - Shutdown the system\n\n"
        "FILESYSTEM:\n"
        "  fat32 init       - Initialize FAT32 filesystem\n"
        "  fat32 ls         - List FAT32 directory\n"
        "  fat32 cat <file> - Read FAT32 file\n\n"
        "CUSTOMIZATION:\n"
        "  color <fg> <bg>  - Set text colors (0-15)\n"
        "  chmod <file> <mode> - Change file permissions\n\n"
        "HELP SYSTEM:\n"
        "  help             - Show quick help\n"
        "  help <command>   - Show help for specific command\n"
        "  help --full      - Show this complete documentation\n\n"
        "EDITOR COMMANDS (vim):\n"
        "  i                - Enter insert mode\n"
        "  ESC              - Return to normal mode\n"
        "  :w               - Save file\n"
        "  :q               - Quit editor\n"
        "  :wq              - Save and quit\n"
        "  h,j,k,l          - Move cursor (left,down,up,right)\n"
        "  x                - Delete character\n"
        "  o                - Open new line below\n"
        "  O                - Open new line above\n\n"
        "NAVIGATION:\n"
        "  Arrow keys       - Navigate in editor and command history\n"
        "  Tab              - Auto-complete commands and filenames\n"
        "  Ctrl+C           - Cancel current operation\n\n"
        "For more information, visit: https://github.com/shamns23/skyos\n\n";
    
    print_with_pagination(help_text);
}

void show_command_help(const char* command) {
    if (strcmp(command, "ls") == 0) show_ls_help();
    else if (strcmp(command, "cd") == 0) show_cd_help();
    else if (strcmp(command, "mkdir") == 0) show_mkdir_help();
    else if (strcmp(command, "touch") == 0) show_touch_help();
    else if (strcmp(command, "cat") == 0) show_cat_help();
    else if (strcmp(command, "vim") == 0) show_vim_help();
    else if (strcmp(command, "fat32") == 0) show_fat32_help();
    else if (strcmp(command, "color") == 0) show_color_help();
    else if (strcmp(command, "debug") == 0) show_debug_help();
    else if (strcmp(command, "tree") == 0) show_tree_help();
    else if (strcmp(command, "write") == 0) show_write_help();
    else if (strcmp(command, "rm") == 0) show_rm_help();
    else if (strcmp(command, "chmod") == 0) show_chmod_help();
    else if (strcmp(command, "pwd") == 0) show_pwd_help();
    else if (strcmp(command, "sysinfo") == 0) show_sysinfo_help();
    else if (strcmp(command, "fastfetch") == 0) show_fastfetch_help();
    else if (strcmp(command, "shutdown") == 0) show_shutdown_help();
    else if (strcmp(command, "clear") == 0) show_clear_help();
    else {
        shell_print_colored("Unknown command: ", COLOR_ERROR, BLACK);
        shell_print_string(command);
        shell_print_string("\nType 'help' for available commands.\n");
    }
}

void print_tree(int dir_index, int depth) {
    for (int i = 0; i < fs_entry_count; i++) {
        if (filesystem[i].parent_dir == dir_index) {
            // Print indentation
            for (int j = 0; j < depth; j++) {
                shell_print_string("  ");
            }
            shell_print_string("|-- ");
            print_entry_colored(&filesystem[i]);
            shell_print_string("\n");
            
            // Recursively print subdirectories
            if (filesystem[i].type == TYPE_DIR) {
                print_tree(i, depth + 1);
            }
        }
    }
}

// Individual help functions
void show_ls_help() {
    shell_print_colored("\n=== ls - List Directory Contents ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: ls [options]\n\n");
    shell_print_string("Options:\n");
    shell_print_string("  -l    Long format (show details)\n");
    shell_print_string("  -a    Show all files (including hidden)\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  ls        List files in current directory\n");
    shell_print_string("  ls -l     List files with details\n");
    shell_print_string("  ls -a     List all files including hidden\n");
    shell_print_string("  ls -la    List all files with details\n\n");
    shell_print_string("Colors:\n");
    shell_print_colored("  Blue", COLOR_DIR, BLACK);
    shell_print_string("      - Directories\n");
    shell_print_colored("  White", COLOR_FILE, BLACK);
    shell_print_string("     - Regular files\n");
    shell_print_colored("  Green", COLOR_EXECUTABLE, BLACK);
    shell_print_string("     - Executable files\n\n");
}

void show_cd_help() {
    shell_print_colored("\n=== cd - Change Directory ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: cd <directory>\n\n");
    shell_print_string("Special directories:\n");
    shell_print_string("  .         Current directory\n");
    shell_print_string("  ..        Parent directory\n");
    shell_print_string("  /         Root directory\n");
    shell_print_string("  ~         Home directory\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  cd home       Change to 'home' directory\n");
    shell_print_string("  cd ..         Go to parent directory\n");
    shell_print_string("  cd /          Go to root directory\n");
    shell_print_string("  cd /home/user Go to absolute path\n\n");
}

void show_mkdir_help() {
    shell_print_colored("\n=== mkdir - Create Directory ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: mkdir [options] <directory>\n\n");
    shell_print_string("Options:\n");
    shell_print_string("  -p    Create parent directories as needed\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  mkdir docs        Create 'docs' directory\n");
    shell_print_string("  mkdir -p a/b/c    Create nested directories\n\n");
    shell_print_string("Note: Directory names cannot contain spaces or special characters.\n\n");
}

void show_touch_help() {
    shell_print_colored("\n=== touch - Create File ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: touch <filename>\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Creates an empty file with the specified name.\n");
    shell_print_string("  If the file already exists, it will not be modified.\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  touch readme.txt    Create empty file\n");
    shell_print_string("  touch config.cfg    Create configuration file\n\n");
    shell_print_string("Note: Filenames cannot contain spaces or special characters.\n\n");
}

void show_cat_help() {
    shell_print_colored("\n=== cat - Display File Contents ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: cat <filename>\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Displays the entire contents of a text file.\n");
    shell_print_string("  For large files, content will be paginated.\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  cat readme.txt      Display file contents\n");
    shell_print_string("  cat /etc/config     Display system config\n\n");
    shell_print_string("Note: Binary files may display garbled text.\n\n");
}

void show_vim_help() {
    shell_print_colored("\n=== vim - Text Editor ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: vim <filename>\n\n");
    shell_print_string("Modes:\n");
    shell_print_string("  Normal Mode  - Navigate and execute commands\n");
    shell_print_string("  Insert Mode  - Type and edit text\n\n");
    shell_print_string("Normal Mode Commands:\n");
    shell_print_string("  i            Enter insert mode\n");
    shell_print_string("  h,j,k,l      Move cursor (left,down,up,right)\n");
    shell_print_string("  x            Delete character under cursor\n");
    shell_print_string("  o            Open new line below\n");
    shell_print_string("  O            Open new line above\n");
    shell_print_string("  :w           Save file\n");
    shell_print_string("  :q           Quit editor\n");
    shell_print_string("  :wq          Save and quit\n\n");
    shell_print_string("Insert Mode:\n");
    shell_print_string("  ESC          Return to normal mode\n");
    shell_print_string("  Arrow keys   Move cursor\n");
    shell_print_string("  Backspace    Delete previous character\n\n");
}

void show_fat32_help() {
    shell_print_colored("\n=== fat32 - FAT32 Filesystem ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: fat32 <command> [arguments]\n\n");
    shell_print_string("Commands:\n");
    shell_print_string("  init         Initialize FAT32 filesystem\n");
    shell_print_string("  ls           List FAT32 directory contents\n");
    shell_print_string("  cat <file>   Display FAT32 file contents\n");
    shell_print_string("  cd <dir>     Change FAT32 directory\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  fat32 init       Initialize filesystem\n");
    shell_print_string("  fat32 ls         List files\n");
    shell_print_string("  fat32 cat boot.cfg  Read file\n\n");
    shell_print_string("Note: FAT32 commands operate on the disk filesystem,\n");
    shell_print_string("      separate from the in-memory filesystem.\n\n");
}

void show_color_help() {
    shell_print_colored("\n=== color - Set Text Colors ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: color <foreground> <background>\n\n");
    shell_print_string("Color codes (0-15):\n");
    shell_print_string("  0=Black    1=Blue     2=Green    3=Cyan\n");
    shell_print_string("  4=Red      5=Magenta  6=Brown    7=Light Gray\n");
    shell_print_string("  8=Dark Gray 9=Light Blue 10=Light Green 11=Light Cyan\n");
    shell_print_string("  12=Light Red 13=Light Magenta 14=Yellow 15=White\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  color 15 0    White text on black background\n");
    shell_print_string("  color 10 0    Green text on black background\n");
    shell_print_string("  color 14 4    Yellow text on red background\n\n");
}

void show_debug_help() {
    shell_print_colored("\n=== debug - Debug Information ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: debug\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Displays system debug information including:\n");
    shell_print_string("  - Memory usage\n");
    shell_print_string("  - Filesystem statistics\n");
    shell_print_string("  - System state\n\n");
    shell_print_string("This command is useful for troubleshooting and\n");
    shell_print_string("understanding system internals.\n\n");
}

void show_tree_help() {
    shell_print_colored("\n=== tree - Directory Tree ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: tree\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Displays the directory structure in a tree format,\n");
    shell_print_string("  showing the hierarchical relationship between\n");
    shell_print_string("  directories and files.\n\n");
    shell_print_string("Example output:\n");
    shell_print_string("  /\n");
    shell_print_string("  |-- home/\n");
    shell_print_string("  |-- bin/\n");
    shell_print_string("  |-- readme.txt\n\n");
}

void show_write_help() {
    shell_print_colored("\n=== write - Write to File ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: write <filename> <content>\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Writes the specified content to a file.\n");
    shell_print_string("  If the file doesn't exist, it will be created.\n");
    shell_print_string("  If it exists, the content will be overwritten.\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  write hello.txt \"Hello World\"\n");
    shell_print_string("  write config.cfg \"setting=value\"\n\n");
}

void show_rm_help() {
    shell_print_colored("\n=== rm - Remove File ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: rm <filename>\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Permanently removes the specified file.\n");
    shell_print_string("  This operation cannot be undone.\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  rm temp.txt      Remove temporary file\n");
    shell_print_string("  rm old_data.log  Remove log file\n\n");
    shell_print_string("Warning: Be careful when using this command!\n\n");
}

void show_chmod_help() {
    shell_print_colored("\n=== chmod - Change Permissions ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: chmod <filename> <mode>\n\n");
    shell_print_string("Permission modes (octal):\n");
    shell_print_string("  644  Read/write for owner, read for others\n");
    shell_print_string("  755  Read/write/execute for owner, read/execute for others\n");
    shell_print_string("  777  Full permissions for everyone\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  chmod script.sh 755    Make script executable\n");
    shell_print_string("  chmod data.txt 644     Standard file permissions\n\n");
}

void show_pwd_help() {
    shell_print_colored("\n=== pwd - Print Working Directory ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: pwd\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Displays the full path of the current working directory.\n\n");
    shell_print_string("Example output:\n");
    shell_print_string("  /home/user\n\n");
}

void show_sysinfo_help() {
    shell_print_colored("\n=== sysinfo - System Information ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: sysinfo\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Displays detailed system information including:\n");
    shell_print_string("  - Operating system version\n");
    shell_print_string("  - Memory information\n");
    shell_print_string("  - Filesystem statistics\n\n");
}

void show_fastfetch_help() {
    shell_print_colored("\n=== fastfetch - Stylized System Info ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: fastfetch\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Displays system information in a colorful,\n");
    shell_print_string("  stylized format similar to neofetch.\n\n");
}

void show_shutdown_help() {
    shell_print_colored("\n=== shutdown - System Shutdown ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: shutdown\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Safely shuts down the operating system.\n");
    shell_print_string("  All unsaved data will be lost.\n\n");
    shell_print_string("Warning: Make sure to save your work before\n");
    shell_print_string("         using this command!\n\n");
}

void show_clear_help() {
    shell_print_colored("\n=== clear - Clear Screen ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: clear\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Clears the terminal screen and moves the\n");
    shell_print_string("  cursor to the top-left corner.\n\n");
}