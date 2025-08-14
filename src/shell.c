#include "shell.h"
#include "display.h"
#include "keyboard.h"
#include "io.h"
#include "string_utils.h"
#include "filesystem.h"
#include <string.h>

void shutdown() {
    shell_print_colored("Shutting down oszoOS...\n", COLOR_INFO, BLACK);
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
    shell_print_colored("\n=== oszoOS v4.1 - Quick Reference ===\n", COLOR_INFO, BLACK);
    shell_print_string(" Files & Directories:\n");
    shell_print_string("  ls [-la]     - List files/directories\n");
    shell_print_string("  cd <dir>     - Change directory\n");
    shell_print_string("  pwd          - Current directory\n");
    shell_print_string("  mkdir <dir>  - Create directory\n");
    shell_print_string("  touch <file> - Create file\n");
    shell_print_string("  cat <file>   - View file contents\n");
    shell_print_string("  rm <file>    - Delete file\n");
    shell_print_string("  write <file> <text> - Write to file\n\n");
    shell_print_string("  System Commands:\n");
    shell_print_string("  clear        - Clear screen\n");
    shell_print_string("  fastfetch    - Stylized system info\n");
    shell_print_string("  memory       - Memory management and info\n");
    shell_print_string("  color <f> <b> - Set colors (0-15)\n");
    shell_print_string("  shutdown     - Shutdown system\n\n");
    shell_print_string(" FAT32 Filesystem:\n");
    shell_print_string("  fat32 init   - Initialize FAT32\n");
    shell_print_string("  fat32 ls     - List FAT32 files\n\n");
    shell_print_string(" Tips: Use Tab for completion, arrows for history\n");
    shell_print_string(" For detailed help: help <command>\n");
    shell_print_string("For full documentation: help --full\n\n");
}

void show_full_help() {
    const char* help_text = 
        "=== oszoOS v4.1 - Complete Documentation ===\n\n"
        "NAVIGATION & DIRECTORY COMMANDS:\n"
        "  ls [options]     - List directory contents\n"
        "    -l             - Long format with permissions, size, date\n"
        "    -a             - Show all files including hidden (.files)\n"
        "    -la            - Combine long format with all files\n"
        "  cd <directory>   - Change current directory\n"
        "    cd ..          - Go to parent directory\n"
        "    cd /           - Go to root directory\n"
        "    cd ~           - Go to home directory (if exists)\n"
        "  pwd              - Print current working directory path\n\n"
        "FILE OPERATIONS:\n"
        "  touch <filename> - Create empty file or update timestamp\n"
        "  cat <filename>   - Display file contents (with pagination)\n"
        "  write <file> <content> - Write/overwrite content to file\n"
        "rm <filename>    - Remove file permanently (WARNING: irreversible)\n"
        "  chmod <file> <mode> - Change file permissions (644, 755, 777)\n\n"
        "DIRECTORY OPERATIONS:\n"
        "  mkdir <dirname>  - Create new directory\n"
        "  mkdir -p <path>  - Create nested directory structure\n\n"
        "SYSTEM & INFORMATION:\n"
        "  clear            - Clear terminal screen\n"
        "  fastfetch        - Show stylized system info with ASCII art\n"
        "  hardware         - Show hardware detection information\n"
        "  memory           - Memory management and statistics\n"
        "  debug            - Display debug info & filesystem stats\n"
        "  shutdown         - Safely shutdown the system\n\n"
        "FAT32 FILESYSTEM:\n"
        "  fat32 init       - Initialize FAT32 filesystem on disk\n"
        "  fat32 info       - Show FAT32 filesystem information\n"
        "  fat32 switch     - Switch between in-memory and FAT32 FS\n"
        "  fat32 ls         - List FAT32 directory contents\n"
        "  fat32 cat <file> - Read file from FAT32 filesystem\n\n"
        "CUSTOMIZATION & SETTINGS:\n"
        "  color <fg> <bg>  - Set text colors (0-15 color codes)\n"
        "    Examples: color 15 0 (white/black), color 10 0 (green/black)\n\n"
        "DEVELOPMENT TOOLS:\n"
        "  run <file.c>     - Simple C code interpreter (prints printf output)\n\n"
        "HELP SYSTEM:\n"
        "  help             - Show quick command reference\n"
        "  help <command>   - Show detailed help for specific command\n"
        "  help --full      - Show this complete documentation\n\n"
        "ADVANCED FEATURES:\n"
        "  Tab completion for commands and filenames\n"
        "  Command history navigation with arrow keys\n"
        "  Color-coded file types (blue=dirs, white=files, green=executables)\n"
        "  Dual filesystem support (in-memory + FAT32)\n"
        "  File permissions system (read/write/execute)\n"
        "  Pagination for long outputs\n\n"
        "TIPS & SHORTCUTS:\n"
        "  Use Tab for auto-completion\n"
        "  Arrow keys for command history\n"
        "  Type partial command + Tab to see suggestions\n"
        "  Use 'help <command>' for detailed command info\n\n"
        "COLOR CODES REFERENCE:\n"
        "  0=Black  1=Blue  2=Green  3=Cyan  4=Red  5=Magenta\n"
        "  6=Brown  7=LightGray  8=DarkGray  9=LightBlue\n"
        "  10=LightGreen  11=LightCyan  12=LightRed  13=LightMagenta\n"
        "  14=Yellow  15=White\n\n"
        "For support: https://github.com/shamns23/oszoos\n"
        "oszoOS - Simple, Fast, Educational Operating System\n\n";
    
    print_with_pagination(help_text);
}

// Command help router function
void show_command_help(const char* command) {
    if (strcmp(command, "ls") == 0) show_ls_help();
    else if (strcmp(command, "cd") == 0) show_cd_help();
    else if (strcmp(command, "mkdir") == 0) show_mkdir_help();
    else if (strcmp(command, "touch") == 0) show_touch_help();
    else if (strcmp(command, "cat") == 0) show_cat_help();
    else if (strcmp(command, "fat32") == 0) show_fat32_help();
    else if (strcmp(command, "color") == 0) show_color_help();
    else if (strcmp(command, "debug") == 0) show_debug_help();
    else if (strcmp(command, "write") == 0) show_write_help();
    else if (strcmp(command, "rm") == 0) show_rm_help();
    else if (strcmp(command, "chmod") == 0) show_chmod_help();
    else if (strcmp(command, "pwd") == 0) show_pwd_help();

    else if (strcmp(command, "fastfetch") == 0) show_fastfetch_help();
    else if (strcmp(command, "hardware") == 0) show_hardware_help();
    else if (strcmp(command, "shutdown") == 0) show_shutdown_help();
    else if (strcmp(command, "clear") == 0) show_clear_help();
    else if (strcmp(command, "memory") == 0) show_memory_help();
    else {
        shell_print_colored("\nUnknown command: ", COLOR_ERROR, BLACK);
        shell_print_colored(command, COLOR_WARNING, BLACK);
        shell_print_string("\n\nAvailable commands:\n");
        shell_print_string("  ls, cd, pwd, mkdir, touch, cat, rm, chmod\n");
    shell_print_string("  write, clear, fastfetch, color\n");
        shell_print_string("  memory, fat32, debug, shutdown\n\n");
        shell_print_string("Use 'help' for quick reference or 'help --full' for complete documentation.\n\n");
    }
}

// Individual help functions
void show_ls_help() {
    shell_print_colored("\n=== ls - List Directory Contents ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: ls [options] [directory]\n\n");
    shell_print_string("Options:\n");
    shell_print_string("  -l    Long format (permissions, size, date, name)\n");
    shell_print_string("  -a    Show all files (including hidden .files)\n");
    shell_print_string("  -la   Combine long format with all files\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  ls           - List current directory\n");
    shell_print_string("  ls -l        - Detailed listing with permissions\n");
    shell_print_string("  ls -a        - Show hidden files too\n");
    shell_print_string("  ls -la       - Detailed list including hidden files\n");
    shell_print_string("  ls /home     - List specific directory\n");
    shell_print_string("  ls ..        - List parent directory\n\n");
    shell_print_string("Description:\n");
    shell_print_string("Lists files and directories with color coding:\n");
    shell_print_string("  Blue = Directories\n");
    shell_print_string("  White = Regular files\n");
    shell_print_string("  Green = Executable files\n\n");
    shell_print_string("Tip: Use Tab completion for directory names\n\n");
}

void show_cd_help() {
    shell_print_colored("\n=== cd - Change Directory ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: cd [directory]\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  cd /home     - Change to /home directory\n");
    shell_print_string("  cd ..        - Go to parent directory\n");
    shell_print_string("  cd /         - Go to root directory\n");
    shell_print_string("  cd ~         - Go to home directory (if exists)\n");
    shell_print_string("  cd           - Go to home directory\n");
    shell_print_string("  cd Documents - Change to Documents folder\n\n");
    shell_print_string("Description:\n");
    shell_print_string("Changes the current working directory to the specified path.\n");
    shell_print_string("Without arguments, attempts to change to home directory.\n\n");
    shell_print_string("Path Types:\n");
    shell_print_string("  Absolute: /home/user (starts with /)\n");
    shell_print_string("  Relative: Documents (relative to current dir)\n");
    shell_print_string("  Special:  .. (parent), . (current), ~ (home)\n\n");
    shell_print_string("Tips:\n");
    shell_print_string("  Use Tab completion for directory names\n");
    shell_print_string("  Use 'pwd' to see current directory\n");
    shell_print_string("  Use 'ls' to see available directories\n\n");
}

void show_mkdir_help() {
    shell_print_colored("\n=== mkdir - Create Directory ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: mkdir [options] <directory>\n\n");
    shell_print_string("Options:\n");
    shell_print_string("  -p    Create parent directories as needed (recursive)\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  mkdir test       - Create 'test' directory\n");
    shell_print_string("  mkdir -p a/b/c   - Create nested directory structure\n");
    shell_print_string("  mkdir docs temp  - Create multiple directories\n");
    shell_print_string("  mkdir /home/new  - Create directory with absolute path\n\n");
    shell_print_string("Description:\n");
    shell_print_string("Creates new directories in the current filesystem.\n");
    shell_print_string("Without -p option, parent directories must already exist.\n");
    shell_print_string("With -p option, creates entire directory path if needed.\n\n");
    shell_print_string("Notes:\n");
    shell_print_string("  Directory names cannot contain special characters\n");
    shell_print_string("  Cannot create directory if name already exists\n");
    shell_print_string("  Use 'ls' to verify directory was created\n\n");
    shell_print_string("Tip: Use Tab completion for existing path parts\n\n");
}

void show_touch_help() {
    shell_print_colored("\n=== touch - Create File ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: touch <filename>\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  touch file.txt   - Create empty text file\n");
    shell_print_string("  touch test.c     - Create C source file\n");
    shell_print_string("  touch readme     - Create file without extension\n");
    shell_print_string("  touch /tmp/log   - Create file with absolute path\n\n");
    shell_print_string("Description:\n");
    shell_print_string("Creates an empty file if it doesn't exist.\n");
    shell_print_string("If file already exists, updates its timestamp.\n");
    shell_print_string("File is created with default permissions (644).\n\n");
    shell_print_string("Notes:\n");
    shell_print_string("  Filename cannot contain special characters\n");
    shell_print_string("  Parent directory must exist\n");
    shell_print_string("  Use 'ls -l' to verify file creation\n\n");
    shell_print_string("Tips:\n");
    shell_print_string("  Use 'write' command to add content to file\n");
    shell_print_string("  Use 'cat' command to view file contents\n");
    shell_print_string("  Use Tab completion for directory paths\n\n");
}

void show_cat_help() {
    shell_print_colored("\n=== cat - Display File Contents ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: cat <filename>\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  cat readme.txt   - Display text file contents\n");
    shell_print_string("  cat config.cfg   - View configuration file\n");
    shell_print_string("  cat /etc/hosts   - Display system file\n");
    shell_print_string("  cat script.sh    - View shell script\n\n");
    shell_print_string("Description:\n");
    shell_print_string("Displays the complete contents of a text file.\n");
    shell_print_string("For large files, output is automatically paginated.\n");
    shell_print_string("Works with both in-memory and FAT32 filesystems.\n\n");
    shell_print_string("Notes:\n");
    shell_print_string("  File must exist and be readable\n");
    shell_print_string("  Binary files may display garbled text\n");
    shell_print_string("  Large files are shown page by page\n\n");
    shell_print_string("Tips:\n");
    shell_print_string("  Use 'ls' to see available files first\n");
    shell_print_string("  Use Tab completion for filenames\n");
    shell_print_string("  Press any key to continue pagination\n\n");
}



void show_fat32_help() {
    shell_print_colored("\n=== fat32 - FAT32 Filesystem Manager ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: fat32 <subcommand> [arguments]\n\n");
    shell_print_string("Subcommands:\n");
    shell_print_string("  init         - Initialize FAT32 filesystem on disk\n");
    shell_print_string("  info         - Show detailed filesystem information\n");
    shell_print_string("  switch       - Toggle between in-memory and FAT32 FS\n");
    shell_print_string("  ls           - List directory contents (FAT32 mode)\n");
    shell_print_string("  cat <file>   - Display file contents from FAT32\n");
    shell_print_string("  cd <dir>     - Change FAT32 directory\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  fat32 init       - Initialize FAT32 on primary disk\n");
    shell_print_string("  fat32 info       - Show disk size, clusters, etc.\n");
    shell_print_string("  fat32 switch     - Switch filesystem mode\n");
    shell_print_string("  fat32 ls         - List files in FAT32 filesystem\n");
    shell_print_string("  fat32 cat readme - Read file from FAT32 disk\n\n");
    shell_print_string("Description:\n");
    shell_print_string("Manages FAT32 filesystem operations and disk access.\n");
    shell_print_string("Provides dual filesystem support: in-memory + FAT32.\n");
    shell_print_string("Allows persistent storage on actual disk hardware.\n\n");
    shell_print_string("Notes:\n");
    shell_print_string("  FAT32 init formats the disk (destructive)\n");
    shell_print_string("  Switch command toggles active filesystem\n");
    shell_print_string("  FAT32 operations work only in FAT32 mode\n\n");
    shell_print_string("Tips:\n");
    shell_print_string("  Use 'fat32 info' to check current mode\n");
    shell_print_string("  Initialize FAT32 before first use\n");
    shell_print_string("  Regular ls/cat work in current mode\n\n");
}

void show_color_help() {
    shell_print_colored("\n=== color - Set Terminal Colors ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: color <foreground> <background>\n\n");
    shell_print_string("Color Codes (0-15):\n");
    shell_print_string("  0=Black      1=Blue       2=Green      3=Cyan\n");
    shell_print_string("  4=Red        5=Magenta    6=Brown      7=LightGray\n");
    shell_print_string("  8=DarkGray   9=LightBlue  10=LightGreen 11=LightCyan\n");
    shell_print_string("  12=LightRed  13=LightMagenta 14=Yellow  15=White\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  color 15 0   - White text on black background (default)\n");
    shell_print_string("  color 10 0   - Green text on black background\n");
    shell_print_string("  color 14 4   - Yellow text on red background\n");
    shell_print_string("  color 11 1   - Light cyan text on blue background\n");
    shell_print_string("  color 0 15   - Black text on white background\n\n");
    shell_print_string("Description:\n");
    shell_print_string("Changes the terminal text and background colors globally.\n");
    shell_print_string("Colors persist until changed again or system restart.\n");
    shell_print_string("Affects all subsequent text output in the terminal.\n\n");
    shell_print_string("Popular Combinations:\n");
    shell_print_string("  color 10 0  - Matrix style (green on black)\n");
    shell_print_string("  color 14 0  - Warning style (yellow on black)\n");
    shell_print_string("  color 12 0  - Error style (red on black)\n");
    shell_print_string("  color 11 0  - Info style (cyan on black)\n\n");
    shell_print_string("Note: Invalid color codes will be ignored\n\n");
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



void show_fastfetch_help() {
    shell_print_colored("\n=== fastfetch - Stylized System Info ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: fastfetch\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Displays system information in a colorful,\n");
    shell_print_string("  stylized format similar to neofetch.\n\n");
}

void show_hardware_help() {
    shell_print_colored("\n=== hardware - Hardware Detection ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: hardware\n\n");
    shell_print_string("Description:\n");
    shell_print_string("  Displays comprehensive hardware information including:\n");
    shell_print_string("  - CPU details (vendor, brand, cores, frequency)\n");
    shell_print_string("  - Memory information (total RAM, usage)\n");
    shell_print_string("  - GPU information (model, memory)\n");
    shell_print_string("  - System architecture and specifications\n\n");
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
    shell_print_string("Clears the terminal screen and moves cursor to top-left.\n\n");
    shell_print_string("Tip: Useful for cleaning up cluttered terminal output.\n\n");
}

void show_memory_help() {
    shell_print_colored("\n=== memory - Memory Management ===\n", COLOR_INFO, BLACK);
    shell_print_string("Usage: memory [subcommand]\n\n");
    shell_print_string("Subcommands:\n");
    shell_print_string("  (none)     - Show memory statistics\n");
    shell_print_string("  dump       - Dump memory blocks information\n");
    shell_print_string("  check      - Check memory integrity\n");
    shell_print_string("  test       - Run allocation test\n\n");
    shell_print_string("Examples:\n");
    shell_print_string("  memory     - Show total/used/free memory\n");
    shell_print_string("  memory dump - Show detailed block information\n");
    shell_print_string("  memory check - Verify memory structure integrity\n");
    shell_print_string("  memory test  - Test allocation/deallocation\n\n");
    shell_print_string("Description:\n");
    shell_print_string("Manages system memory with simple malloc/free implementation.\n");
    shell_print_string("Provides memory statistics and debugging capabilities.\n\n");
    shell_print_string("Tip: Use 'memory check' if experiencing memory issues\n\n");
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
                    shell_print_colored("oszoOS", COLOR_SUCCESS, BLACK);
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

// Helper function to find matching commands
int find_matching_commands(const char* prefix, char matches[][128], int max_matches) {
    const char* commands[] = {"help", "ls", "cd", "cat", "write", "mkdir", "rm", "clear", "pwd", "edit", "fastfetch", "info", "reboot", "shutdown", "version"};
    int count = sizeof(commands) / sizeof(commands[0]);
    int match_count = 0;
    
    for (int i = 0; i < count && match_count < max_matches; i++) {
        if (strncmp(commands[i], prefix, strlen(prefix)) == 0) {
            SAFE_STRCPY(matches[match_count], commands[i], 128);
            match_count++;
        }
    }
    
    return match_count;
}

// Helper function to find matching files
int find_matching_files(const char* prefix, char matches[][128], int max_matches, int want_dir) {
    int match_count = 0;
    
    for (int i = 0; i < fs_entry_count && match_count < max_matches; i++) {
        if (filesystem[i].parent_dir == current_dir) {
            if (want_dir && filesystem[i].type != TYPE_DIR) continue;
            if (strncmp(filesystem[i].name, prefix, strlen(prefix)) == 0) {
                SAFE_STRCPY(matches[match_count], filesystem[i].name, 128);
                if (filesystem[i].type == TYPE_DIR) {
                    SAFE_STRCAT(matches[match_count], "/", 128);
                }
                match_count++;
            }
        }
    }
    
    return match_count;
}