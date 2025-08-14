#include "editor.h"
#include "filesystem.h"
#include "display.h"
#include "string_utils.h"
#include "shell.h"

// Global editor variables
char editor_buffer[MAX_CONTENT];
int editor_file_index = -1;
int editor_cursor_pos = 0;
int editor_line = 0;
int editor_col = 0;

// Initialize the editor
void init_editor() {
    editor_buffer[0] = '\0';
    editor_file_index = -1;
    editor_cursor_pos = 0;
    editor_line = 0;
    editor_col = 0;
}

// Open a file in the editor
int editor_open(const char* filename) {
    int file_index = resolve_path_full(filename, 1);
    if (file_index == -1 || filesystem[file_index].type != TYPE_FILE) {
        shell_print_colored("File not found: ", COLOR_ERROR, BLACK);
        shell_print_string(filename);
        shell_print_string("\n");
        return -1;
    }
    
    SAFE_STRCPY(editor_buffer, filesystem[file_index].content, MAX_CONTENT);
    editor_file_index = file_index;
    editor_cursor_pos = 0;
    editor_line = 0;
    editor_col = 0;
    
    shell_print_colored("Opened: ", COLOR_SUCCESS, BLACK);
    shell_print_string(filename);
    shell_print_string("\n");
    
    return 0;
}

// Save the current file
int editor_save() {
    if (editor_file_index == -1) {
        shell_print_colored("No file is open\n", COLOR_ERROR, BLACK);
        return -1;
    }
    
    SAFE_STRCPY(filesystem[editor_file_index].content, editor_buffer, MAX_CONTENT);
    filesystem[editor_file_index].size = 0;
    while (editor_buffer[filesystem[editor_file_index].size]) {
        filesystem[editor_file_index].size++;
    }
    
    shell_print_colored("File saved\n", COLOR_SUCCESS, BLACK);
    return 0;
}

// Save and close the editor
int editor_save_and_close() {
    if (editor_file_index == -1) {
        shell_print_colored("No file is open\n", COLOR_ERROR, BLACK);
        return -1;
    }
    
    SAFE_STRCPY(filesystem[editor_file_index].content, editor_buffer, MAX_CONTENT);
    filesystem[editor_file_index].size = 0;
    while (editor_buffer[filesystem[editor_file_index].size]) {
        filesystem[editor_file_index].size++;
    }
    
    shell_print_colored("File saved and editor closed\n", COLOR_SUCCESS, BLACK);
    editor_file_index = -1;
    editor_buffer[0] = '\0';
    editor_cursor_pos = 0;
    editor_line = 0;
    editor_col = 0;
    
    return 0;
}

// Display the current file content
void editor_display() {
    if (editor_file_index == -1) {
        shell_print_colored("No file is open\n", COLOR_ERROR, BLACK);
        return;
    }
    
    shell_print_colored("--- Editor ---\n", COLOR_INFO, BLACK);
    shell_print_string(editor_buffer);
    shell_print_colored("\n--- End ---\n", COLOR_INFO, BLACK);
}

// Run the editor
void run_editor(const char* filename) {
    if (editor_open(filename) != 0) {
        return;
    }
    
    shell_print_colored("Simple Text Editor\n", COLOR_INFO, BLACK);
    shell_print_colored("Commands: :w save, :q quit, :wq save and quit\n", COLOR_INFO, BLACK);
    shell_print_colored("--- File Content ---\n", COLOR_INFO, BLACK);
    
    int running = 1;
    char input_buffer[256];
    
    while (running) {
        editor_display();
        shell_print_colored("\neditor> ", COLOR_INFO, BLACK);
        
        readline(input_buffer, sizeof(input_buffer));
        
        if (strcmp(input_buffer, ":q") == 0) {
            running = 0;
        } else if (strcmp(input_buffer, ":w") == 0) {
            editor_save();
        } else if (strcmp(input_buffer, ":wq") == 0) {
            editor_save_and_close();
            running = 0;
        } else if (strcmp(input_buffer, "help") == 0) {
            shell_print_colored("Commands:\n", COLOR_INFO, BLACK);
            shell_print_colored(":w  - save file\n", COLOR_INFO, BLACK);
            shell_print_colored(":q  - quit editor\n", COLOR_INFO, BLACK);
            shell_print_colored(":wq - save and quit\n", COLOR_INFO, BLACK);
            shell_print_colored("help - show this help\n", COLOR_INFO, BLACK);
        } else {
            shell_print_colored("Unknown command. Type 'help' for help.\n", COLOR_WARNING, BLACK);
        }
    }
}