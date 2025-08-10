#ifndef HARDWARE_DETECTION_H
#define HARDWARE_DETECTION_H

#include <stdint.h>

#define MAX_PCI_DEVICES 64

// CPU Features structure
typedef struct {
    uint8_t sse;
    uint8_t sse2;
    uint8_t sse3;
    uint8_t ssse3;
    uint8_t sse4_1;
    uint8_t sse4_2;
} CPUFeatures;

// CPU Information structure
typedef struct {
    char vendor[13];        // CPU vendor string (12 chars + null)
    char brand[49];         // CPU brand string (48 chars + null)
    uint8_t family;
    uint8_t model;
    uint8_t stepping;
    CPUFeatures features;
} CPUInfo;

// Memory Information structure
typedef struct {
    uint64_t total_ram;     // Total RAM in bytes
    uint64_t available_ram; // Available RAM in bytes
    uint64_t used_ram;      // Used RAM in bytes
} MemoryInfo;

// PCI Device structure
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t class_code;
    uint8_t subclass;
} PCIDevice;

// PCI Information structure
typedef struct {
    uint8_t device_count;
    PCIDevice devices[MAX_PCI_DEVICES];
} PCIInfo;

// Main Hardware Information structure
typedef struct {
    CPUInfo cpu;
    MemoryInfo memory;
    PCIInfo pci;
} HardwareInfo;

// Function declarations
void hardware_detection_init(void);
HardwareInfo* get_hardware_info(void);
void display_hardware_info(void);
void detect_cpu_info(void);
void detect_memory_info(void);
void scan_pci_devices(void);
void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

#endif // HARDWARE_DETECTION_H