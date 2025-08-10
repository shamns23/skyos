#ifndef FASTFETCH_H
#define FASTFETCH_H

#include <stdint.h>

// Structure for system information
typedef struct {
    char os_name[64];
    char kernel_version[32];
    char hostname[64];
    char architecture[16];
    uint64_t uptime;
    
    struct {
        char vendor[64];
        char brand[128];
        uint32_t cores;
        uint32_t threads;
        uint32_t frequency;
    } cpu;
    
    struct {
        uint64_t total_ram;
        uint64_t used_ram;
    } memory;
    
    struct {
        char vendor[64];
        char model[64];
        uint32_t resolution_x;
        uint32_t resolution_y;
    } gpu;
    
    struct {
        uint64_t total_size;
        uint64_t used_size;
        char type[16];
    } storage;
    
    struct {
        int current_charge;
        char status[32];
    } battery;
    
    struct {
        int cpu_temp;
        int gpu_temp;
    } thermal;
} SystemInfo;

// Function declarations
void display_fastfetch_style(void);
void display_logo(void);
void display_system_info(void);
void format_memory_size(uint64_t bytes, char* buffer);
void fastfetch_init(void);
SystemInfo get_system_info(void);

#endif // FASTFETCH_H