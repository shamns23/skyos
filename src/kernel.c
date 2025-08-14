#include <stddef.h>
#include "string_utils.h"
#include "display.h"
#include "keyboard.h"
#include "filesystem.h"
#include "memory.h"

#include "shell.h"
#include "command_handler.h"
// Global variables for kernel
// Display variables moved to display.c
// Editor variables moved to editor.c
#define PIT_FREQ 100
volatile unsigned int system_ticks = 0;
extern void irq0_handler();

// Timer interrupt handler
void timer_handler() {
    system_ticks++;
}
void shell_print_string(const char* str);
// Help function declarations moved to shell.h
// Display and I/O functions are now in separate modules
// Keyboard functions moved to keyboard.c
// Filesystem structures and functions moved to filesystem.c
// Filesystem functions moved to filesystem.c
// delay function moved to io.c
// shutdown function moved to shell.c
void process_cmd(char* cmd);
void readline(char* buffer, int max_len);
int find_matching_commands(const char* prefix, char matches[][128], int max_matches);
int find_matching_files(const char* prefix, char matches[][128], int max_matches, int only_files);
// Editor function declarations moved to editor.h
void kernel_main();
/* الدالة الرئيسية للنواة */
int main() {
    // Call kernel_main to start the system
    kernel_main();
    return 0;
}

void kernel_main() {
    // Write directly to VGA buffer for debugging
    char *video = (char*)0xB8000;
    const char *msg1 = "oszoOS v4.1 - Kernel Started!";
    
    // Clear screen
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = 0x07;
    }
    
    // Write startup message
    for (int i = 0; msg1[i] != '\0'; i++) {
        video[i * 2] = msg1[i];
        video[i * 2 + 1] = 0x0F;
    }
    
    // Initialize basic systems step by step
    const char *msg2 = "Initializing keyboard...";
    for (int i = 0; msg2[i] != '\0'; i++) {
        video[160 + i * 2] = msg2[i];
        video[160 + i * 2 + 1] = 0x07;
    }
    init_keyboard();
    
    const char *msg3 = "Initializing filesystem...";
    for (int i = 0; msg3[i] != '\0'; i++) {
        video[320 + i * 2] = msg3[i];
        video[320 + i * 2 + 1] = 0x07;
    }
    init_filesystem();
    
    const char *msg4 = "Initializing memory...";
    for (int i = 0; msg4[i] != '\0'; i++) {
        video[480 + i * 2] = msg4[i];
        video[480 + i * 2 + 1] = 0x07;
    }
    memory_init();
    
    const char *msg5 = "System ready! Type 'help' for commands.";
    for (int i = 0; msg5[i] != '\0'; i++) {
        video[640 + i * 2] = msg5[i];
        video[640 + i * 2 + 1] = 0x0A;
    }
    
    // Simple command loop
    char cmd_buffer[128];
    while (1) {
        const char *prompt = "oszoOS > ";
        for (int i = 0; prompt[i] != '\0'; i++) {
            video[800 + i * 2] = prompt[i];
            video[800 + i * 2 + 1] = 0x0E;
        }
        
        readline(cmd_buffer, sizeof(cmd_buffer));
        if (cmd_buffer[0] != '\0') {
            process_cmd(cmd_buffer);
        }
    }
}
void process_cmd(char* cmd) {
    // Use the new improved command handler
    int handled = process_command(cmd);
    
    // If command was handled by new system, return
    if (handled) {
        return;
    }
    
    // Keep legacy commands that haven't been migrated yet
    char* saveptr;
    char* command = strtok_r(cmd, " ", &saveptr);
    if (command == NULL) { 
        return;
    }
    
    // Check if command was already handled by new system
    if (strcmp(command, "clear") == 0 ||
        strcmp(command, "help") == 0 ||
        strcmp(command, "ls") == 0 ||
        strcmp(command, "cd") == 0 ||
        strcmp(command, "pwd") == 0 ||
        strcmp(command, "mkdir") == 0 ||
        strcmp(command, "cat") == 0 ||
        strcmp(command, "rm") == 0 ||
        strcmp(command, "chmod") == 0 ||
        strcmp(command, "touch") == 0 ||
        strcmp(command, "color") == 0 ||
        strcmp(command, "memory") == 0 ||
        strcmp(command, "fastfetch") == 0 ||
        strcmp(command, "hardware") == 0 ||
        strcmp(command, "hwinfo") == 0 ||
        strcmp(command, "write") == 0 ||
        strcmp(command, "run") == 0 ||
        strcmp(command, "shutdown") == 0 ||
        strcmp(command, "fat32") == 0 ||
        strcmp(command, "debug") == 0) {
        return; // Already handled by new system
    }
    
    // Legacy commands (to be migrated) - This section should be empty now
    // All commands have been migrated to the new command handler
    
    // Show error for unknown commands
    shell_print_colored("Error: ", COLOR_ERROR, BLACK);
    shell_print_colored("Unknown command: ", COLOR_ERROR, BLACK);
    shell_print_colored(command, COLOR_WARNING, BLACK);
    shell_print_colored("\nType ", WHITE, BLACK);
    shell_print_colored("help", COLOR_INFO, BLACK);
    shell_print_colored(" for available commands.\n", WHITE, BLACK);
}

// Helper function to find matching commands - moved to shell.c

// Helper function to format time

// Helper function to find matching commands


// Helper function to format time
void format_time(uint32_t seconds, char* buffer) {
    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    
    if (hours > 0) {
        itoa(hours, buffer);
        SAFE_STRCAT(buffer, "h ", 32);
    }
    
    char temp[16];
    itoa(minutes, temp);
    SAFE_STRCAT(buffer, temp, 32);
    SAFE_STRCAT(buffer, "m ", 32);
    
    itoa(secs, temp);
    SAFE_STRCAT(buffer, temp, 32);
    SAFE_STRCAT(buffer, "s", 32);
}
