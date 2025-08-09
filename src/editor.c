#include "editor.h"
#include "display.h"
#include "keyboard.h"
#include "filesystem.h"
#include "string_utils.h"

// Editor global variables
char editor_buffer[MAX_CONTENT];
int editor_cursor = 0;
int editor_mode = EDITOR_MODE_NORMAL;
int editor_file_index = -1;
char editor_cmdline[32] = "";
int editor_cmdline_active = 0;

typedef struct {
    int should_exit;
} EditorState;

EditorState editor_state = {0};

void editor_display() {
    clear_screen();
    
    // Display file content
    shell_print_colored("=== SkyOS Text Editor ===", COLOR_INFO, BLACK);
    shell_print_string("\n");
    
    if (editor_file_index >= 0) {
        shell_print_colored("File: ", COLOR_PROMPT, BLACK);
        shell_print_colored(filesystem[editor_file_index].name, COLOR_FILE, BLACK);
        shell_print_string("\n");
    }
    
    shell_print_string("\n");
    
    // Display buffer content with cursor
    for (size_t i = 0; i < my_strlen(editor_buffer); i++) {
        if ((int)i == editor_cursor && editor_mode == EDITOR_MODE_INSERT) {
            shell_print_colored("|", COLOR_WARNING, BLACK);
        }
        shell_print_char(editor_buffer[i]);
    }
    
    if ((size_t)editor_cursor >= my_strlen(editor_buffer) && editor_mode == EDITOR_MODE_INSERT) {
        shell_print_colored("|", COLOR_WARNING, BLACK);
    }
    
    // Display status line
    shell_print_string("\n\n");
    shell_print_colored("--- ", COLOR_PROMPT, BLACK);
    if (editor_mode == EDITOR_MODE_NORMAL) {
        shell_print_colored("NORMAL", COLOR_SUCCESS, BLACK);
    } else {
        shell_print_colored("INSERT", COLOR_WARNING, BLACK);
    }
    shell_print_colored(" ---", COLOR_PROMPT, BLACK);
    
    // Display command line if active
    if (editor_cmdline_active) {
        shell_print_string("\n:");
        shell_print_string(editor_cmdline);
    }
    
    shell_print_string("\n\nCommands: i=insert, ESC=normal, :w=save, :q=quit, :wq=save&quit");
}

void editor_init(int file_index) {
    editor_file_index = file_index;
    editor_cursor = 0;
    editor_mode = EDITOR_MODE_NORMAL;
    editor_cmdline_active = 0;
    editor_cmdline[0] = '\0';
    editor_state.should_exit = 0;
    
    if (file_index >= 0) {
        strcpy(editor_buffer, filesystem[file_index].content);
    } else {
        editor_buffer[0] = '\0';
    }
}

void editor_process_key(int key) {
    if (editor_cmdline_active) {
        if (key == '\n') {
            // Process command
            if (strcmp(editor_cmdline, "w") == 0) {
                // Save file
                if (editor_file_index >= 0) {
                    strcpy(filesystem[editor_file_index].content, editor_buffer);
                    filesystem[editor_file_index].size = my_strlen(editor_buffer);
                }
            } else if (strcmp(editor_cmdline, "q") == 0) {
                // Quit
                editor_state.should_exit = 1;
            } else if (strcmp(editor_cmdline, "wq") == 0) {
                // Save and quit
                if (editor_file_index >= 0) {
                    strcpy(filesystem[editor_file_index].content, editor_buffer);
                    filesystem[editor_file_index].size = my_strlen(editor_buffer);
                }
                editor_state.should_exit = 1;
            }
            editor_cmdline_active = 0;
            editor_cmdline[0] = '\0';
        } else if (key == KEY_ESC_CODE) {
            editor_cmdline_active = 0;
            editor_cmdline[0] = '\0';
        } else if (key == '\b') {
            int len = my_strlen(editor_cmdline);
            if (len > 0) {
                editor_cmdline[len - 1] = '\0';
            }
        } else if (key >= 32 && key < 127) {
            size_t len = my_strlen(editor_cmdline);
            if (len < sizeof(editor_cmdline) - 1) {
                editor_cmdline[len] = key;
                editor_cmdline[len + 1] = '\0';
            }
        }
        return;
    }
    
    if (editor_mode == EDITOR_MODE_NORMAL) {
        switch (key) {
            case 'i':
                editor_mode = EDITOR_MODE_INSERT;
                break;
            case ':':
                editor_cmdline_active = 1;
                editor_cmdline[0] = '\0';
                break;
            case 'h':
            case ARROW_LEFT:
                if (editor_cursor > 0) editor_cursor--;
                break;
            case 'l':
            case ARROW_RIGHT:
                if ((size_t)editor_cursor < my_strlen(editor_buffer)) editor_cursor++;
                break;
            case 'j':
            case ARROW_DOWN:
                // Move down one line (simplified)
                while ((size_t)editor_cursor < my_strlen(editor_buffer) && editor_buffer[editor_cursor] != '\n') {
                    editor_cursor++;
                }
                if ((size_t)editor_cursor < my_strlen(editor_buffer)) editor_cursor++;
                break;
            case 'k':
            case ARROW_UP:
                // Move up one line (simplified)
                if (editor_cursor > 0) {
                    editor_cursor--;
                    while (editor_cursor > 0 && editor_buffer[editor_cursor] != '\n') {
                        editor_cursor--;
                    }
                }
                break;
            case '0':
            case KEY_HOME_CODE:
                // Move to beginning of line
                while (editor_cursor > 0 && editor_buffer[editor_cursor - 1] != '\n') {
                    editor_cursor--;
                }
                break;
            case '$':
            case KEY_END_CODE:
                // Move to end of line
                while ((size_t)editor_cursor < my_strlen(editor_buffer) && editor_buffer[editor_cursor] != '\n') {
                    editor_cursor++;
                }
                break;
            case 'x':
                // Delete character
                if ((size_t)editor_cursor < my_strlen(editor_buffer)) {
                    size_t len = my_strlen(editor_buffer);
                    for (size_t i = editor_cursor; i < len; i++) {
                        editor_buffer[i] = editor_buffer[i + 1];
                    }
                }
                break;
            case 'o':
                // Open new line below
                {
                    int len = my_strlen(editor_buffer);
                    // Move to end of current line
                    while (editor_cursor < len && editor_buffer[editor_cursor] != '\n') {
                        editor_cursor++;
                    }
                    // Insert newline
                    if (len < MAX_CONTENT - 1) {
                        for (int i = len; i > editor_cursor; i--) {
                            editor_buffer[i] = editor_buffer[i - 1];
                        }
                        editor_buffer[editor_cursor] = '\n';
                        editor_buffer[len + 1] = '\0';
                        editor_cursor++;
                    }
                    editor_mode = EDITOR_MODE_INSERT;
                }
                break;
            case 'O':
                // Open new line above
                {
                    // Move to beginning of current line
                    while (editor_cursor > 0 && editor_buffer[editor_cursor - 1] != '\n') {
                        editor_cursor--;
                    }
                    // Insert newline
                    int len = my_strlen(editor_buffer);
                    if (len < MAX_CONTENT - 1) {
                        for (int i = len; i >= editor_cursor; i--) {
                            editor_buffer[i + 1] = editor_buffer[i];
                        }
                        editor_buffer[editor_cursor] = '\n';
                        editor_buffer[len + 1] = '\0';
                    }
                    editor_mode = EDITOR_MODE_INSERT;
                }
                break;
        }
    } else if (editor_mode == EDITOR_MODE_INSERT) {
        switch (key) {
            case KEY_ESC_CODE:
                editor_mode = EDITOR_MODE_NORMAL;
                break;
            case '\b':
                if (editor_cursor > 0) {
                    editor_cursor--;
                    int len = my_strlen(editor_buffer);
                    for (int i = editor_cursor; i < len; i++) {
                        editor_buffer[i] = editor_buffer[i + 1];
                    }
                }
                break;
            case ARROW_LEFT:
                if (editor_cursor > 0) editor_cursor--;
                break;
            case ARROW_RIGHT:
                if ((size_t)editor_cursor < my_strlen(editor_buffer)) editor_cursor++;
                break;
            case ARROW_UP:
                // Move up one line (simplified)
                if (editor_cursor > 0) {
                    editor_cursor--;
                    while (editor_cursor > 0 && editor_buffer[editor_cursor] != '\n') {
                        editor_cursor--;
                    }
                }
                break;
            case ARROW_DOWN:
                // Move down one line (simplified)
                while ((size_t)editor_cursor < my_strlen(editor_buffer) && editor_buffer[editor_cursor] != '\n') {
                    editor_cursor++;
                }
                if ((size_t)editor_cursor < my_strlen(editor_buffer)) editor_cursor++;
                break;
            default:
                if ((key >= 32 && key < 127) || key == '\n' || key == '\t') {
                    int len = my_strlen(editor_buffer);
                    if (len < MAX_CONTENT - 1) {
                        for (int i = len; i > editor_cursor; i--) {
                            editor_buffer[i] = editor_buffer[i - 1];
                        }
                        editor_buffer[editor_cursor] = key;
                        editor_buffer[len + 1] = '\0';
                        editor_cursor++;
                    }
                }
                break;
        }
    }
}

void editor_run(char* filename) {
    int file_index = find_entry(filename);
    if (file_index == -1) {
        // Create new file
        file_index = create_file(filename, "");
        if (file_index == -1) {
            shell_print_colored("Error: Could not create file\n", COLOR_ERROR, BLACK);
            return;
        }
    }
    
    if (filesystem[file_index].type != TYPE_FILE) {
        shell_print_colored("Error: Not a file\n", COLOR_ERROR, BLACK);
        return;
    }
    
    editor_init(file_index);
    
    while (!editor_state.should_exit) {
        editor_display();
        int key = get_char();
        editor_process_key(key);
    }
    
    clear_screen();
}