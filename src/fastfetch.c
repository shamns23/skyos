#include "fastfetch.h"
#include "display.h"
#include "string_utils.h"

// Get system information
SystemInfo get_system_info() {
    SystemInfo info;
    
    // Basic system info
    strcpy(info.os_name, "oszoOS v4.1");
    strcpy(info.kernel_version, "4.1.0");
    strcpy(info.hostname, "oszoOS-PC");
    strcpy(info.architecture, "x86_32");
    info.uptime = 3600; // 1 hour for demo
    
    // CPU info
    strcpy(info.cpu.vendor, "Intel");
    strcpy(info.cpu.brand, "Intel Core i7-9700K");
    info.cpu.cores = 8;
    info.cpu.threads = 8;
    info.cpu.frequency = 3600;
    
    // Memory info
    info.memory.total_ram = 4 * 1024 * 1024 * 1024ULL; // 4GB
    info.memory.used_ram = 2 * 1024 * 1024 * 1024ULL;  // 2GB
    
    // GPU info
    strcpy(info.gpu.vendor, "NVIDIA");
    strcpy(info.gpu.model, "GeForce GTX 1660 Ti");
    info.gpu.resolution_x = 1920;
    info.gpu.resolution_y = 1080;
    
    // Storage info
    info.storage.total_size = 512 * 1024 * 1024 * 1024ULL; // 512GB
    info.storage.used_size = 128 * 1024 * 1024 * 1024ULL;  // 128GB
    strcpy(info.storage.type, "SSD");
    
    // Battery info
    info.battery.current_charge = 85;
    strcpy(info.battery.status, "Charging");
    
    // Thermal info
    info.thermal.cpu_temp = 45;
    info.thermal.gpu_temp = 50;
    
    return info;
}

// Display enhanced fastfetch-style system information
void display_fastfetch_style() {
    SystemInfo info = get_system_info();
    char buffer[64];
    
    // Enhanced ASCII Logo Banner
    shell_print_colored(" _______  _______  _______  _______  _______  _______   \n", LIGHT_BLUE, BLACK);
    shell_print_colored("|       ||       ||       ||       ||       ||       |  \n", LIGHT_BLUE, BLACK);
    shell_print_colored("|   _   ||  _____||____   ||   _   ||   _   ||  _____|  \n", LIGHT_BLUE, BLACK);
    shell_print_colored("|  | |  || |_____  ____|  ||  | |  ||  | |  || |_____   \n", LIGHT_BLUE, BLACK);
    shell_print_colored("|  |_|  ||_____  || ______||  |_|  ||  |_|  ||_____  |  \n", LIGHT_BLUE, BLACK);
    shell_print_colored("|       | _____| || |_____ |       ||       | _____| |  \n", LIGHT_BLUE, BLACK);
    shell_print_colored("|_______||_______||_______||_______||_______||_______|  \n", LIGHT_BLUE, BLACK);
    
    shell_print_colored("\n", BLACK, BLACK);
    
    // Enhanced System Information
    shell_print_colored("OS: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.os_name);
    shell_print_string(" ");
    shell_print_string(info.kernel_version);
    shell_print_char('\n');
    
    shell_print_colored("Host: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.hostname);
    shell_print_char('\n');
    
    shell_print_colored("Kernel: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.architecture);
    shell_print_char('\n');
    
    shell_print_colored("Uptime: ", LIGHT_GREEN, BLACK);
    itoa(info.uptime / 3600, buffer);
    shell_print_string(buffer);
    shell_print_string("h ");
    itoa((info.uptime % 3600) / 60, buffer);
    shell_print_string(buffer);
    shell_print_string("m\n");
    
    shell_print_colored("CPU: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.cpu.brand);
    shell_print_string(" (");
    itoa(info.cpu.cores, buffer);
    shell_print_string(buffer);
    shell_print_string("c/");
    itoa(info.cpu.threads, buffer);
    shell_print_string(buffer);
    shell_print_string("t @ ");
    itoa(info.cpu.frequency, buffer);
    shell_print_string(buffer);
    shell_print_string(" MHz)\n");
    
    shell_print_colored("Memory: ", LIGHT_GREEN, BLACK);
    format_memory_size(info.memory.used_ram, buffer);
    shell_print_string(buffer);
    shell_print_string(" / ");
    format_memory_size(info.memory.total_ram, buffer);
    shell_print_string(buffer);
    shell_print_string(" (");
    uint32_t mem_percent = 0;
    if (info.memory.total_ram > 0) {
        mem_percent = (uint32_t)(((long double)info.memory.used_ram * 100.0L) / (long double)info.memory.total_ram);
    }
    itoa((int)mem_percent, buffer);
    shell_print_string(buffer);
    shell_print_string("%)\n");
    
    shell_print_colored("GPU: ", LIGHT_GREEN, BLACK);
    shell_print_string(info.gpu.vendor);
    shell_print_string(" ");
    shell_print_string(info.gpu.model);
    shell_print_string(" @ ");
    itoa(info.gpu.resolution_x, buffer);
    shell_print_string(buffer);
    shell_print_string("x");
    itoa(info.gpu.resolution_y, buffer);
    shell_print_string(buffer);
    shell_print_char('\n');
    
    shell_print_colored("Storage: ", LIGHT_GREEN, BLACK);
    format_memory_size(info.storage.used_size, buffer);
    shell_print_string(buffer);
    shell_print_string(" / ");
    format_memory_size(info.storage.total_size, buffer);
    shell_print_string(buffer);
    shell_print_string(" (");
    shell_print_string(info.storage.type);
    shell_print_string(")\n");
    
    shell_print_colored("Battery: ", LIGHT_GREEN, BLACK);
    itoa(info.battery.current_charge, buffer);
    shell_print_string(buffer);
    shell_print_string("%");
    if (info.battery.status[0] != '\0') {
        shell_print_string(" (");
        shell_print_string(info.battery.status);
        shell_print_string(")");
    }
    shell_print_char('\n');
    
    shell_print_colored("Thermal: ", LIGHT_GREEN, BLACK);
    itoa(info.thermal.cpu_temp, buffer);
    shell_print_string(buffer);
    shell_print_string("°C CPU / ");
    itoa(info.thermal.gpu_temp, buffer);
    shell_print_string(buffer);
    shell_print_string("°C GPU\n");
    
    shell_print_colored("Shell: ", LIGHT_GREEN, BLACK);
    shell_print_string("Custom Shell\n");
    
    shell_print_colored("Terminal: ", LIGHT_GREEN, BLACK);
    shell_print_string("Virtual Terminal\n");
}

// Format memory size to human-readable format
void format_memory_size(uint64_t bytes, char* buffer) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = (double)bytes;
    
    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        itoa((int)size, buffer);
    } else {
        // Simple float to string conversion with overflow protection
        int whole = (int)size;
        int fraction = (int)((size - (double)whole) * 10.0);
        
        if (fraction < 0) fraction = 0;
        if (fraction > 9) fraction = 9;
        
        char whole_str[32];
        char fraction_str[8];
        itoa(whole, whole_str);
        itoa(fraction, fraction_str);
        
        strcpy(buffer, whole_str);
        strcat(buffer, ".");
        strcat(buffer, fraction_str);
    }
    
    strcat(buffer, units[unit_index]);
}

// Initialize fastfetch system
void fastfetch_init() {
    // Initialize fastfetch-specific components
    // No specific initialization needed as get_system_info provides all data
}