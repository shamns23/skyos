#ifndef SYSINFO_H
#define SYSINFO_H

#include <stdint.h>

// System information structures
typedef struct {
    char vendor[13];
    char brand[49];
    uint32_t cores;
    uint32_t threads;
    uint32_t frequency;
} CPUInfo;

typedef struct {
    uint64_t total_ram;
    uint64_t available_ram;
    uint64_t total_swap;
    uint64_t available_swap;
} MemoryInfo;

typedef struct {
    char vendor[32];
    char model[64];
    uint64_t vram;
} GPUInfo;

typedef struct {
    CPUInfo cpu;
    MemoryInfo memory;
    GPUInfo gpu;
    char os_name[32];
    char kernel_version[32];
    char architecture[16];
} SystemInfo;

// Function declarations
void sysinfo_init(void);
SystemInfo get_system_info(void);
void display_detailed_sysinfo(void);
void display_fastfetch_style(void);

#endif // SYSINFO_H