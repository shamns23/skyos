#include "sysinfo.h"
#include "string_utils.h"
#include "display.h"
#include <stdint.h>



// Get CPU vendor using CPUID instruction
static void get_cpu_vendor(char* vendor) {
    uint32_t eax, ebx, ecx, edx;
    
    // CPUID function 0: Get vendor ID
    asm volatile (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (0)
    );
    
    // Extract vendor string from ebx, edx, ecx
    *((uint32_t*)vendor) = ebx;
    *((uint32_t*)(vendor + 4)) = edx;
    *((uint32_t*)(vendor + 8)) = ecx;
    vendor[12] = '\0';
}

// Get CPU brand string using CPUID
static void get_cpu_brand(char* brand) {
    uint32_t eax, ebx, ecx, edx;
    
    // CPUID function 0x80000002-0x80000004: Get processor brand string
    for (int i = 0; i < 3; i++) {
        asm volatile (
            "cpuid"
            : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
            : "a" (0x80000002 + i)
        );
        
        *((uint32_t*)(brand + i * 16)) = eax;
        *((uint32_t*)(brand + i * 16 + 4)) = ebx;
        *((uint32_t*)(brand + i * 16 + 8)) = ecx;
        *((uint32_t*)(brand + i * 16 + 12)) = edx;
    }
    brand[48] = '\0';
}

// Get CPU cores and threads count
static uint32_t get_cpu_cores() {
    uint32_t eax, ebx, ecx, edx;
    
    // CPUID function 1: Get processor info
    asm volatile (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (1)
    );
    
    // Extract core count from EBX[23:16]
    return (ebx >> 16) & 0xFF;
}

// Get CPU frequency (simplified - returns base frequency)
static uint32_t get_cpu_frequency() {
    uint32_t eax, ebx, ecx, edx;
    
    // CPUID function 0x16: Get processor frequency info
    asm volatile (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (0x16)
    );
    
    // EAX contains base frequency in MHz
    return eax;
}

// Get total system memory using BIOS interrupt
static uint64_t get_total_memory() {
    uint32_t mem_kb = 0;
    
    // Use BIOS interrupt 0x15, EAX=0xE820 to get memory map
    // For now, return a simulated value based on detected memory
    return 1024 * 1024 * 1024; // 1GB as default
}

// Detect GPU information (simplified for x86)
static void detect_gpu(GPUInfo* gpu) {
    // For x86 systems, detect basic VGA info
    strcpy(gpu->vendor, "Intel/VGA");
    strcpy(gpu->model, "Standard VGA Graphics Adapter");
    gpu->vram = 8 * 1024 * 1024; // 8MB VRAM
}

// Initialize system information detection
void sysinfo_init() {
    // This function would initialize hardware detection
    // For now, we'll use static values for demonstration
}

// Get complete system information
SystemInfo get_system_info() {
    SystemInfo info;
    
    // OS Information
    strcpy(info.os_name, "SkyOS");
    strcpy(info.kernel_version, "4.1.0");
    strcpy(info.architecture, "x86_64");
    
    // CPU Information
    get_cpu_vendor(info.cpu.vendor);
    get_cpu_brand(info.cpu.brand);
    info.cpu.cores = get_cpu_cores();
    info.cpu.threads = info.cpu.cores * 2; // Assume hyperthreading
    info.cpu.frequency = get_cpu_frequency();
    
    // Memory Information
    info.memory.total_ram = get_total_memory();
    info.memory.available_ram = info.memory.total_ram - (64 * 1024 * 1024); // Simulate usage
    info.memory.total_swap = 512 * 1024 * 1024; // 512MB swap
    info.memory.available_swap = info.memory.total_swap;
    
    // GPU Information
    detect_gpu(&info.gpu);
    
    return info;
}

// Format memory size to human-readable format
static void format_memory_size(uint64_t bytes, char* buffer) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = (double)bytes;
    
    while (size >= 1024 && unit_index < 4) {
        size /= 1024;
        unit_index++;
    }
    
    if (unit_index == 0) {
        itoa((int)size, buffer);
    } else {
        // Simple float to string conversion
        int whole = (int)size;
        int fraction = (int)((size - whole) * 100);
        
        char whole_str[16];
        char fraction_str[16];
        itoa(whole, whole_str);
        itoa(fraction, fraction_str);
        
        strcpy(buffer, whole_str);
        strcat(buffer, ".");
        strcat(buffer, fraction_str);
    }
    
    strcat(buffer, units[unit_index]);
}

// Display detailed system information
void display_detailed_sysinfo() {
    SystemInfo info = get_system_info();
    char buffer[64];
    
    shell_print_colored("═══════════════════════════════════════\n", LIGHT_CYAN, BLACK);
    shell_print_colored("         SYSTEM INFORMATION\n", LIGHT_GREEN, BLACK);
    shell_print_colored("═══════════════════════════════════════\n", LIGHT_CYAN, BLACK);
    
    // OS Information
    shell_print_colored("OS: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.os_name);
    shell_print_string(" ");
    shell_print_string(info.kernel_version);
    shell_print_char('\n');
    
    shell_print_colored("Architecture: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.architecture);
    shell_print_char('\n');
    
    // CPU Information
    shell_print_colored("CPU: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.cpu.brand);
    shell_print_char('\n');
    
    shell_print_colored("CPU Vendor: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.cpu.vendor);
    shell_print_char('\n');
    
    shell_print_colored("CPU Cores: ", LIGHT_GREEN, BLACK);
    itoa(info.cpu.cores, buffer);
    shell_print_string(buffer);
    shell_print_char('\n');
    
    shell_print_colored("CPU Threads: ", LIGHT_GREEN, BLACK);
    itoa(info.cpu.threads, buffer);
    shell_print_string(buffer);
    shell_print_char('\n');
    
    shell_print_colored("CPU Frequency: ", LIGHT_GREEN, BLACK);
    itoa(info.cpu.frequency, buffer);
    shell_print_string(buffer);
    shell_print_string(" MHz\n");
    
    // Memory Information
    shell_print_colored("Total RAM: ", LIGHT_GREEN, BLACK);
    format_memory_size(info.memory.total_ram, buffer);
    shell_print_string(buffer);
    shell_print_char('\n');
    
    shell_print_colored("Available RAM: ", LIGHT_GREEN, BLACK);
    format_memory_size(info.memory.available_ram, buffer);
    shell_print_string(buffer);
    shell_print_char('\n');
    
    shell_print_colored("Total Swap: ", LIGHT_GREEN, BLACK);
    format_memory_size(info.memory.total_swap, buffer);
    shell_print_string(buffer);
    shell_print_char('\n');
    
    shell_print_colored("Available Swap: ", LIGHT_GREEN, BLACK);
    format_memory_size(info.memory.available_swap, buffer);
    shell_print_string(buffer);
    shell_print_char('\n');
    
    // GPU Information
    shell_print_colored("GPU Vendor: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.gpu.vendor);
    shell_print_char('\n');
    
    shell_print_colored("GPU Model: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.gpu.model);
    shell_print_char('\n');
    
    shell_print_colored("GPU VRAM: ", LIGHT_GREEN, BLACK);
    format_memory_size(info.gpu.vram, buffer);
    shell_print_string(buffer);
    shell_print_char('\n');
    
    shell_print_colored("═══════════════════════════════════════\n", LIGHT_CYAN, BLACK);
}

// Display enhanced fastfetch-style system info
void display_fastfetch_style() {
    SystemInfo info = get_system_info();
    char buffer[64];
    
    // ASCII Art Logo
    shell_print_colored("        .--.\n", LIGHT_CYAN, BLACK);
    shell_print_colored("       |o_o |\n", LIGHT_CYAN, BLACK);
    shell_print_colored("       |:_/ |\n", LIGHT_CYAN, BLACK);
    shell_print_colored("      //   \ \\\n", LIGHT_CYAN, BLACK);
    shell_print_colored("     (|     | )\n", LIGHT_CYAN, BLACK);
    shell_print_colored("    /'\_   _/`\ \n", LIGHT_CYAN, BLACK);
    shell_print_colored("    \___)=(___/\n", LIGHT_CYAN, BLACK);
    shell_print_char('\n');
    
    // System Information in fastfetch format
    shell_print_colored("OS: ", LIGHT_GREEN, BLACK);
    shell_print_colored(info.os_name, WHITE, BLACK);
    shell_print_string(" ");
    shell_print_colored(info.kernel_version, WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Host: ", LIGHT_GREEN, BLACK);
    shell_print_colored("SkyOS Virtual Machine", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Kernel: ", LIGHT_GREEN, BLACK);
    shell_print_colored("SkyOS Kernel", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Uptime: ", LIGHT_GREEN, BLACK);
    shell_print_colored("System Ready", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Packages: ", LIGHT_GREEN, BLACK);
    shell_print_colored("Built-in", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Shell: ", LIGHT_GREEN, BLACK);
    shell_print_colored("SkyOS Shell", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Resolution: ", LIGHT_GREEN, BLACK);
    shell_print_colored("80x25", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("CPU: ", LIGHT_GREEN, BLACK);
    shell_print_colored(info.cpu.brand, WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("GPU: ", LIGHT_GREEN, BLACK);
    shell_print_colored(info.gpu.vendor, WHITE, BLACK);
    shell_print_string(" ");
    shell_print_colored(info.gpu.model, WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Memory: ", LIGHT_GREEN, BLACK);
    format_memory_size(info.memory.available_ram, buffer);
    shell_print_colored(buffer, WHITE, BLACK);
    shell_print_string(" / ");
    format_memory_size(info.memory.total_ram, buffer);
    shell_print_colored(buffer, WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Disk (/): ", LIGHT_GREEN, BLACK);
    shell_print_colored("512MB", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Terminal: ", LIGHT_GREEN, BLACK);
    shell_print_colored("VGA Console", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Theme: ", LIGHT_GREEN, BLACK);
    shell_print_colored("VGA Color Palette", WHITE, BLACK);
    shell_print_char('\n');
    
    shell_print_colored("Locale: ", LIGHT_GREEN, BLACK);
    shell_print_colored("en_US.UTF-8", WHITE, BLACK);
    shell_print_char('\n');
}