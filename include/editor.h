#ifndef EDITOR_H
#define EDITOR_H

#include "filesystem.h"

// Editor modes
#define EDITOR_MODE_NORMAL 0
#define EDITOR_MODE_INSERT 1

// Editor global variables
extern char editor_buffer[MAX_CONTENT];
extern int editor_cursor;
extern int editor_mode;
extern int editor_file_index;
extern char editor_cmdline[32];
extern int editor_cmdline_active;

// Function declarations
void editor_display();
void editor_init(int file_index);
void editor_process_key(int key);
void editor_run(char* filename);

#endif // EDITOR_H