#include "fastfetch.h"
#include "display.h"
#include "string_utils.h"
#include "hardware_detection.h"

// Get system information
SystemInfo get_system_info() {
    SystemInfo info;
    HardwareInfo* hw_info = get_hardware_info();
    
    // Basic system info
    SAFE_STRCPY(info.os_name, "oszoOS v4.1", sizeof(info.os_name));
    SAFE_STRCPY(info.kernel_version, "4.1.0", sizeof(info.kernel_version));
    SAFE_STRCPY(info.hostname, "oszoOS-PC", sizeof(info.hostname));
    SAFE_STRCPY(info.architecture, "x86_32", sizeof(info.architecture));
    info.uptime = 3600; // 1 hour for demo
    
    // CPU info from real hardware detection - using all available data
    if (hw_info && hw_info->cpu.vendor[0] != '\0') {
        SAFE_STRCPY(info.cpu.vendor, hw_info->cpu.vendor, sizeof(info.cpu.vendor));
        if (hw_info->cpu.brand[0] != '\0') {
            SAFE_STRCPY(info.cpu.brand, hw_info->cpu.brand, sizeof(info.cpu.brand));
        } else {
            SAFE_STRCPY(info.cpu.brand, "Unknown CPU", sizeof(info.cpu.brand));
        }
        info.cpu.cores = 1; // Single core detection
        info.cpu.threads = 1;
        info.cpu.frequency = 0; // Frequency not detected in current implementation
    } else {
        SAFE_STRCPY(info.cpu.vendor, "Unknown", sizeof(info.cpu.vendor));
        SAFE_STRCPY(info.cpu.brand, "Unknown CPU", sizeof(info.cpu.brand));
        info.cpu.cores = 1;
        info.cpu.threads = 1;
        info.cpu.frequency = 0;
    }
    
    // Memory info from real hardware detection - using exact values
    if (hw_info) {
        info.memory.total_ram = hw_info->memory.total_ram;
        info.memory.used_ram = hw_info->memory.used_ram;
    } else {
        info.memory.total_ram = 256 * 1024 * 1024ULL; // 256MB fallback
        info.memory.used_ram = 128 * 1024 * 1024ULL;  // 128MB fallback
    }
    
    // GPU info - detect from PCI devices if available
    SAFE_STRCPY(info.gpu.vendor, "Unknown", sizeof(info.gpu.vendor));
    SAFE_STRCPY(info.gpu.model, "Unknown GPU", sizeof(info.gpu.model));
    if (hw_info && hw_info->pci.device_count > 0) {
        // Look for VGA/Display controller (class 0x03)
        for (int i = 0; i < hw_info->pci.device_count; i++) {
            if (hw_info->pci.devices[i].class_code == 0x03) {
                // Found display controller
                if (hw_info->pci.devices[i].vendor_id == 0x1013) {
                    SAFE_STRCPY(info.gpu.vendor, "Cirrus Logic", sizeof(info.gpu.vendor));
                    SAFE_STRCPY(info.gpu.model, "GD 5446", sizeof(info.gpu.model));
                } else if (hw_info->pci.devices[i].vendor_id == 0x1234) {
                    SAFE_STRCPY(info.gpu.vendor, "QEMU", sizeof(info.gpu.vendor));
                    SAFE_STRCPY(info.gpu.model, "Virtual VGA", sizeof(info.gpu.model));
                } else {
                    SAFE_STRCPY(info.gpu.vendor, "Unknown", sizeof(info.gpu.vendor));
                    SAFE_STRCPY(info.gpu.model, "PCI Display", sizeof(info.gpu.model));
                }
                break;
            }
        }
    } else {
        // Fallback for QEMU
        SAFE_STRCPY(info.gpu.vendor, "Cirrus Logic", sizeof(info.gpu.vendor));
        SAFE_STRCPY(info.gpu.model, "GD 5446", sizeof(info.gpu.model));
    }
    info.gpu.resolution_x = 1024;
    info.gpu.resolution_y = 768;
    
    // Storage info - realistic for our OS
    info.storage.total_size = 1440 * 1024ULL; // 1.44MB floppy equivalent
    info.storage.used_size = 256 * 1024ULL;   // 256KB used
    SAFE_STRCPY(info.storage.type, "FAT32", sizeof(info.storage.type));
    
    // Remove battery and thermal info as they're not applicable for virtual machines
    
    return info;
}

// Helper function to format memory sizes
void format_memory_size(uint64_t size, char* buffer) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    long double display_size = (long double)size;
    
    while (display_size >= 1024.0L && unit_index < 4) {
        display_size /= 1024.0L;
        unit_index++;
    }
    
    if (unit_index == 0) {
        itoa((uint32_t)display_size, buffer);
    } else {
        // Format with one decimal place
        uint32_t whole = (uint32_t)display_size;
        uint32_t fraction = (uint32_t)((display_size - whole) * 10.0L);
        
        char whole_str[16];
        itoa(whole, whole_str);
        char fraction_str[8];
        itoa(fraction, fraction_str);
        
        SAFE_STRCPY(buffer, whole_str, 32);
        SAFE_STRCAT(buffer, ".", 32);
        SAFE_STRCAT(buffer, fraction_str, 32);
        SAFE_STRCAT(buffer, units[unit_index], 32);
    }
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
    shell_print_string(info.cpu.vendor);
    shell_print_string(")\n");
    
    // Additional CPU details from hardware detection
    HardwareInfo* hw_info = get_hardware_info();
    if (hw_info && hw_info->cpu.vendor[0] != '\0') {
        shell_print_colored("CPU Details: ", LIGHT_GREEN, BLACK);
        shell_print_string("Family ");
        itoa(hw_info->cpu.family, buffer);
        shell_print_string(buffer);
        shell_print_string(" Model ");
        itoa(hw_info->cpu.model, buffer);
        shell_print_string(buffer);
        shell_print_string(" Stepping ");
        itoa(hw_info->cpu.stepping, buffer);
        shell_print_string(buffer);
        shell_print_string("\n");
        
        shell_print_colored("CPU Features: ", LIGHT_GREEN, BLACK);
        if (hw_info->cpu.features.sse) shell_print_string("SSE ");
        if (hw_info->cpu.features.sse2) shell_print_string("SSE2 ");
        if (hw_info->cpu.features.sse3) shell_print_string("SSE3 ");
        if (hw_info->cpu.features.ssse3) shell_print_string("SSSE3 ");
        if (hw_info->cpu.features.sse4_1) shell_print_string("SSE4.1 ");
        if (hw_info->cpu.features.sse4_2) shell_print_string("SSE4.2 ");
        shell_print_string("\n");
    }
    
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
    
    // Display PCI devices information from hardware detection
    if (hw_info && hw_info->pci.device_count > 0) {
        shell_print_colored("PCI Devices: ", LIGHT_GREEN, BLACK);
        itoa(hw_info->pci.device_count, buffer);
        shell_print_string(buffer);
        shell_print_string(" found\n");
        
        // Show first few important devices
        int shown = 0;
        for (int i = 0; i < hw_info->pci.device_count && shown < 3; i++) {
            if (hw_info->pci.devices[i].class_code == 0x03 || 
                hw_info->pci.devices[i].class_code == 0x02 || 
                hw_info->pci.devices[i].class_code == 0x01) {
                shell_print_colored("  - ", LIGHT_GREEN, BLACK);
                shell_print_string("Vendor 0x");
                itoa_hex(hw_info->pci.devices[i].vendor_id, buffer);
                shell_print_string(buffer);
                shell_print_string(" Device 0x");
                itoa_hex(hw_info->pci.devices[i].device_id, buffer);
                shell_print_string(buffer);
                shell_print_char('\n');
                shown++;
            }
        }
    }
    
    shell_print_colored("\n", BLACK, BLACK);
}



// Initialize fastfetch system
void fastfetch_init() {
    // Initialize fastfetch-specific components
    // No specific initialization needed as get_system_info provides all data
}