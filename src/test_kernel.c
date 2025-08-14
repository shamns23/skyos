// Minimal test kernel to verify multiboot works
void test_main(void) {
    // Write directly to VGA text buffer
    char *video = (char*)0xB8000;
    const char *message = "oszoOS Test Kernel Loaded!";
    int i = 0;
    
    // Clear screen
    for (int j = 0; j < 80 * 25 * 2; j += 2) {
        video[j] = ' ';
        video[j + 1] = 0x07; // White on black
    }
    
    // Write message
    while (message[i] != '\0') {
        video[i * 2] = message[i];
        video[i * 2 + 1] = 0x0F; // Bright white on black
        i++;
    }
    
    // Infinite loop
    while (1) {
        asm volatile("hlt");
    }
}