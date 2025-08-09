#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

// Command handler function pointer type
typedef void (*CommandHandler)(char* args);

// Command entry structure
typedef struct {
    const char* name;
    CommandHandler handler;
} CommandEntry;

// Main command processor function
// Returns 1 if command was handled, 0 if not found
int process_command(char* cmd);

#endif // COMMAND_HANDLER_H