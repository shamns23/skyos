#ifndef SYSINFO_H
#define SYSINFO_H

#include <stdint.h>

// System information structures
typedef struct {
    char vendor[13];
    char brand[49];
    char family[32];
    char stepping[16];
    uint32_t cores;
    uint32_t threads;
    uint32_t frequency;
    uint32_t max_frequency;
    uint32_t cache_l1;
    uint32_t cache_l2;
    uint32_t cache_l3;
} CPUInfo;

typedef struct {
    uint64_t total_ram;
    uint64_t available_ram;
    uint64_t used_ram;
    uint64_t total_swap;
    uint64_t available_swap;
    uint64_t used_swap;
    uint32_t memory_slots;
    uint32_t memory_speed;
} MemoryInfo;

typedef struct {
    char vendor[32];
    char model[64];
    char driver[32];
    uint64_t vram;
    uint32_t resolution_x;
    uint32_t resolution_y;
    uint32_t refresh_rate;
    char api[16];
} GPUInfo;

typedef struct {
    char vendor[32];
    char model[64];
    char type[16]; // SSD, HDD, NVMe
    uint64_t total_size;
    uint64_t used_size;
    uint64_t free_size;
    uint32_t read_speed;
    uint32_t write_speed;
    char interface[16]; // SATA, NVMe, USB
} StorageInfo;

typedef struct {
    char vendor[32];
    char model[32];
    uint32_t capacity; // mAh
    uint32_t current_charge;
    uint32_t charge_cycles;
    char status[16]; // Charging, Discharging, Full
    uint32_t voltage;
} BatteryInfo;

typedef struct {
    uint32_t cpu_temp;
    uint32_t gpu_temp;
    uint32_t system_temp;
    uint32_t fan_speed;
    char thermal_status[16];
} ThermalInfo;

typedef struct {
    CPUInfo cpu;
    MemoryInfo memory;
    GPUInfo gpu;
    StorageInfo storage;
    BatteryInfo battery;
    ThermalInfo thermal;
    char os_name[32];
    char kernel_version[32];
    char architecture[16];
    char hostname[64];
    uint32_t uptime;
    uint32_t boot_time;
} SystemInfo;

// Function declarations
void sysinfo_init(void);
SystemInfo get_system_info(void);
void display_detailed_sysinfo(void);
void display_fastfetch_style(void);

#endif // SYSINFO_H