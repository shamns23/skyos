#include "hardware_detection.h"
#include "io.h"
#include "display.h"
#include "string_utils.h"
#include <stdint.h>

// Global hardware info structure
HardwareInfo hw_info;

// CPUID function to get CPU information
void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

// Detect CPU information using CPUID - improved version with debugging
void detect_cpu_info() {
    uint32_t eax, ebx, ecx, edx;
    
    // Clear CPU info first
    memset(&hw_info.cpu, 0, sizeof(CPUInfo));
    
    shell_print_string("[CPU] Starting CPU detection...\n");
    
    // Check if CPUID is supported
    uint32_t flags_before, flags_after;
    __asm__ volatile (
        "pushfl\n\t"
        "popl %0\n\t"
        "movl %0, %1\n\t"
        "xorl $0x200000, %0\n\t"
        "pushl %0\n\t"
        "popfl\n\t"
        "pushfl\n\t"
        "popl %0\n\t"
        "pushl %1\n\t"
        "popfl"
        : "=&r" (flags_after), "=&r" (flags_before)
        :
        : "cc"
    );
    
    if ((flags_before ^ flags_after) & 0x200000) {
        // CPUID is supported
        
        // Get CPU vendor string
        cpuid(0, &eax, &ebx, &ecx, &edx);
        
        shell_print_string("[CPU] CPUID max value: ");
        print_int(eax);
        shell_print_string("\n");
        
        // Store vendor string (EBX, EDX, ECX order)
        *((uint32_t*)&hw_info.cpu.vendor[0]) = ebx;
        *((uint32_t*)&hw_info.cpu.vendor[4]) = edx;
        *((uint32_t*)&hw_info.cpu.vendor[8]) = ecx;
        hw_info.cpu.vendor[12] = '\0';
        
        shell_print_string("[CPU] Vendor: ");
        shell_print_string(hw_info.cpu.vendor);
        shell_print_string("\n");
        
        // Get CPU features and model info
        cpuid(1, &eax, &ebx, &ecx, &edx);
        
        // Extract CPU family, model, stepping with extended values
        uint8_t base_family = (eax >> 8) & 0xF;
        uint8_t base_model = (eax >> 4) & 0xF;
        
        if (base_family == 0xF) {
            hw_info.cpu.family = base_family + ((eax >> 20) & 0xFF);
        } else {
            hw_info.cpu.family = base_family;
        }
        
        if (base_family == 0x6 || base_family == 0xF) {
            hw_info.cpu.model = base_model + (((eax >> 16) & 0xF) << 4);
        } else {
            hw_info.cpu.model = base_model;
        }
        
        hw_info.cpu.stepping = eax & 0xF;
        
        shell_print_string("[CPU] Family: ");
        print_int(hw_info.cpu.family);
        shell_print_string(" Model: ");
        print_int(hw_info.cpu.model);
        shell_print_string(" Stepping: ");
        print_int(hw_info.cpu.stepping);
        shell_print_string("\n");
        
        // Extract features
        hw_info.cpu.features.sse = (edx >> 25) & 1;
        hw_info.cpu.features.sse2 = (edx >> 26) & 1;
        hw_info.cpu.features.sse3 = ecx & 1;
        hw_info.cpu.features.ssse3 = (ecx >> 9) & 1;
        hw_info.cpu.features.sse4_1 = (ecx >> 19) & 1;
        hw_info.cpu.features.sse4_2 = (ecx >> 20) & 1;
        
        shell_print_string("[CPU] Features - SSE: ");
        print_int(hw_info.cpu.features.sse);
        shell_print_string(" SSE2: ");
        print_int(hw_info.cpu.features.sse2);
        shell_print_string(" SSE3: ");
        print_int(hw_info.cpu.features.sse3);
        shell_print_string("\n");
        
        // Get CPU brand string if available
        cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
        
        if (eax >= 0x80000004) {
            // CPU brand string is available
            char *brand_ptr = hw_info.cpu.brand;
            
            for (uint32_t i = 0x80000002; i <= 0x80000004; i++) {
                cpuid(i, &eax, &ebx, &ecx, &edx);
                *((uint32_t*)brand_ptr) = eax; brand_ptr += 4;
                *((uint32_t*)brand_ptr) = ebx; brand_ptr += 4;
                *((uint32_t*)brand_ptr) = ecx; brand_ptr += 4;
                *((uint32_t*)brand_ptr) = edx; brand_ptr += 4;
            }
            hw_info.cpu.brand[48] = '\0';
            
            shell_print_string("[CPU] Brand: ");
            shell_print_string(hw_info.cpu.brand);
            shell_print_string("\n");
        } else {
            my_strncpy(hw_info.cpu.brand, "Unknown CPU", 48);
            shell_print_string("[CPU] Brand string not available\n");
        }
    } else {
        // CPUID not supported
        my_strncpy(hw_info.cpu.vendor, "Unknown", 12);
        my_strncpy(hw_info.cpu.brand, "Legacy CPU", 48);
        hw_info.cpu.family = 0;
        hw_info.cpu.model = 0;
        hw_info.cpu.stepping = 0;
        shell_print_string("[CPU] CPUID not supported\n");
    }
}

// Detect memory information using BIOS memory map
void detect_memory_info() {
    // Improved memory detection using multiple methods
    
    // Method 1: Read from CMOS extended memory registers
    outb(0x70, 0x17); // Low byte of extended memory
    uint8_t low_mem = inb(0x71);
    outb(0x70, 0x18); // High byte of extended memory  
    uint8_t high_mem = inb(0x71);
    
    uint32_t extended_mem = ((uint32_t)high_mem << 8) | low_mem; // in KB
    
    // Debug CMOS values
    shell_print_string("[Memory] CMOS Extended Memory: ");
    print_int(extended_mem);
    shell_print_string(" KB\n");
    
    // Method 2: Use QEMU memory size (simplified for now)
    // In real hardware, we'd use BIOS memory map
    uint32_t total_memory = 256 * 1024 * 1024; // 256MB (QEMU default)
    
    // If CMOS gives valid value, use it
    if (extended_mem > 0 && extended_mem < 65536) {
        total_memory = extended_mem * 1024 + 1024 * 1024; // Extended + 1MB
    }
    
    hw_info.memory.total_ram = total_memory;
    hw_info.memory.available_ram = total_memory - (2 * 1024 * 1024); // Reserve 2MB
    hw_info.memory.used_ram = 2 * 1024 * 1024; // Kernel + system
    
    shell_print_string("[Memory] Total RAM: ");
    print_int(total_memory / (1024 * 1024));
    shell_print_string(" MB\n");
}

// PCI configuration space access - improved version
uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = 0x80000000 | ((uint32_t)bus << 16) | ((uint32_t)device << 11) | 
                      ((uint32_t)function << 8) | (offset & 0xFC);
    
    outl(0xCF8, address);
    return inl(0xCFC);
}

// Read 16-bit value from PCI config space
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t data = pci_config_read(bus, device, function, offset & 0xFC);
    return (data >> ((offset & 2) * 8)) & 0xFFFF;
}

// Read 8-bit value from PCI config space
uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t data = pci_config_read(bus, device, function, offset & 0xFC);
    return (data >> ((offset & 3) * 8)) & 0xFF;
}

// Scan PCI devices - improved version with debugging
void scan_pci_devices() {
    hw_info.pci.device_count = 0;
    
    shell_print_string("[PCI] Starting PCI device scan...\n");
    
    // Check if PCI is available by testing configuration mechanism
    outl(0xCF8, 0x80000000);
    if (inl(0xCF8) != 0x80000000) {
        shell_print_string("[PCI] PCI not available\n");
        return; // PCI not available
    }
    
    shell_print_string("[PCI] PCI available, starting scan...\n");
    
    // Scan all possible PCI locations
    for (int bus = 0; bus < 256 && hw_info.pci.device_count < MAX_PCI_DEVICES; bus++) {
        for (int device = 0; device < 32 && hw_info.pci.device_count < MAX_PCI_DEVICES; device++) {
            // Check if device exists (function 0 must exist if device exists)
            uint16_t vendor_id = pci_config_read16(bus, device, 0, 0);
            
            if (vendor_id == 0xFFFF) {
                continue; // No device at this location
            }
            
            // Check header type to determine if it's multi-function
            uint8_t header_type = pci_config_read8(bus, device, 0, 0x0E);
            uint8_t max_functions = (header_type & 0x80) ? 8 : 1;
            
            for (int function = 0; function < max_functions && hw_info.pci.device_count < MAX_PCI_DEVICES; function++) {
                vendor_id = pci_config_read16(bus, device, function, 0);
                
                if (vendor_id != 0xFFFF) {
                    // Valid device found
                    PCIDevice *pci_dev = &hw_info.pci.devices[hw_info.pci.device_count];
                    
                    pci_dev->vendor_id = vendor_id;
                    pci_dev->device_id = pci_config_read16(bus, device, function, 2);
                    pci_dev->bus = bus;
                    pci_dev->device = device;
                    pci_dev->function = function;
                    
                    // Get class code and subclass
                    pci_dev->class_code = pci_config_read8(bus, device, function, 0x0B);
                    pci_dev->subclass = pci_config_read8(bus, device, function, 0x0A);
                    
                    shell_print_string("[PCI] Found device: Bus ");
                    print_int(bus);
                    shell_print_string(" Device ");
                    print_int(device);
                    shell_print_string(" Function ");
                    print_int(function);
                    shell_print_string(" Vendor 0x");
                    print_hex(vendor_id);
                    shell_print_string(" Device 0x");
                    print_hex(pci_dev->device_id);
                    shell_print_string("\n");
                    
                    hw_info.pci.device_count++;
                }
            }
        }
    }
    
    shell_print_string("[PCI] Scan complete. Found ");
    print_int(hw_info.pci.device_count);
    shell_print_string(" devices\n");
}

// Initialize hardware detection
void hardware_detection_init() {
    // Clear hardware info structure
    memset(&hw_info, 0, sizeof(HardwareInfo));
    
    // Debug output
    shell_print_string("[Hardware Detection] Starting hardware detection...\n");
    
    // Detect hardware components
    detect_cpu_info();
    shell_print_string("[Hardware Detection] CPU detection complete\n");
    
    detect_memory_info();
    shell_print_string("[Hardware Detection] Memory detection complete\n");
    
    scan_pci_devices();
    shell_print_string("[Hardware Detection] PCI scan complete\n");
}

// Get hardware information
HardwareInfo* get_hardware_info() {
    return &hw_info;
}

// Display hardware information
void display_hardware_info() {
    shell_print_string("\n=== Hardware Information ===\n");
    
    // Display CPU information
    shell_print_string("CPU: ");
    shell_print_string(hw_info.cpu.brand);
    shell_print_string("\n");
    
    shell_print_string("Vendor: ");
    shell_print_string(hw_info.cpu.vendor);
    shell_print_string("\n");
    
    shell_print_string("Family: ");
    print_int(hw_info.cpu.family);
    shell_print_string(" Model: ");
    print_int(hw_info.cpu.model);
    shell_print_string("\n");
    
    // Display memory information
    shell_print_string("Memory: ");
    print_int(hw_info.memory.total_ram / (1024 * 1024));
    shell_print_string(" MB\n");
    
    shell_print_string("Available: ");
    print_int(hw_info.memory.available_ram / (1024 * 1024));
    shell_print_string(" MB\n");
    
    // Display PCI devices
    shell_print_string("PCI Devices: ");
    print_int(hw_info.pci.device_count);
    shell_print_string(" found\n");
    
    for (int i = 0; i < hw_info.pci.device_count; i++) {
        PCIDevice *dev = &hw_info.pci.devices[i];
        shell_print_string("  Bus ");
        print_int(dev->bus);
        shell_print_string(" Device ");
        print_int(dev->device);
        shell_print_string(": ");
        print_hex(dev->vendor_id);
        shell_print_string(":");
        print_hex(dev->device_id);
        shell_print_string("\n");
    }
    
    shell_print_string("============================\n");
}