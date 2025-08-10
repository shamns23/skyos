#include "memory.h"
#include "display.h"
#include "string_utils.h"

// Memory pool - static allocation
static unsigned char memory_pool[MEMORY_POOL_SIZE];
static memory_block_t* free_list = NULL;
static int memory_initialized = 0;

// Initialize memory management
void memory_init(void) {
    if (memory_initialized) return;
    
    // Create initial free block covering entire pool
    free_list = (memory_block_t*)memory_pool;
    free_list->size = MEMORY_POOL_SIZE - sizeof(memory_block_t);
    free_list->free = 1;
    free_list->next = NULL;
    free_list->prev = NULL;
    
    memory_initialized = 1;
}

// Allocate memory
void* malloc(size_t size) {
    if (!memory_initialized) {
        memory_init();
    }
    
    if (size == 0) return NULL;
    
    // Align size to 16 bytes
    size = (size + 15) & ~15;
    
    memory_block_t* current = free_list;
    
    // Find first suitable block using first-fit algorithm
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // Found suitable block
            if (current->size >= size + sizeof(memory_block_t) + MIN_BLOCK_SIZE) {
                // Split block if there's enough space
                memory_block_t* new_block = (memory_block_t*)((unsigned char*)current + sizeof(memory_block_t) + size);
                new_block->size = current->size - size - sizeof(memory_block_t);
                new_block->free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_block;
                }
                
                current->size = size;
                current->next = new_block;
            }
            
            current->free = 0;
            return (unsigned char*)current + sizeof(memory_block_t);
        }
        current = current->next;
    }
    
    // No suitable block found
    shell_print_string("malloc: Out of memory!\n");
    return NULL;
}

// Free allocated memory
void free(void* ptr) {
    if (!ptr) return;
    
    memory_block_t* block = (memory_block_t*)((unsigned char*)ptr - sizeof(memory_block_t));
    
    if ((unsigned char*)block < memory_pool || 
        (unsigned char*)block >= memory_pool + MEMORY_POOL_SIZE) {
        shell_print_string("free: Invalid pointer!\n");
        return;
    }
    
    block->free = 1;
    
    // Coalesce with next block if free
    if (block->next && block->next->free) {
        block->size += sizeof(memory_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    // Coalesce with previous block if free
    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(memory_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

// Allocate and zero memory
void* calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = malloc(total_size);
    
    if (ptr) {
        unsigned char* byte_ptr = (unsigned char*)ptr;
        for (size_t i = 0; i < total_size; i++) {
            byte_ptr[i] = 0;
        }
    }
    
    return ptr;
}

// Reallocate memory
void* realloc(void* ptr, size_t new_size) {
    if (!ptr) return malloc(new_size);
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    
    memory_block_t* block = (memory_block_t*)((unsigned char*)ptr - sizeof(memory_block_t));
    
    if (block->size >= new_size) {
        return ptr; // Current block is large enough
    }
    
    void* new_ptr = malloc(new_size);
    if (new_ptr) {
        // Copy old data
        unsigned char* src = (unsigned char*)ptr;
        unsigned char* dst = (unsigned char*)new_ptr;
        size_t copy_size = (block->size < new_size) ? block->size : new_size;
        
        for (size_t i = 0; i < copy_size; i++) {
            dst[i] = src[i];
        }
        
        free(ptr);
    }
    
    return new_ptr;
}

// Get free memory size
size_t memory_get_free(void) {
    if (!memory_initialized) memory_init();
    
    size_t free_size = 0;
    memory_block_t* current = free_list;
    
    while (current != NULL) {
        if (current->free) {
            free_size += current->size;
        }
        current = current->next;
    }
    
    return free_size;
}

// Get used memory size
size_t memory_get_used(void) {
    if (!memory_initialized) memory_init();
    
    size_t used_size = 0;
    memory_block_t* current = (memory_block_t*)memory_pool;
    
    while ((unsigned char*)current < memory_pool + MEMORY_POOL_SIZE) {
        if (!current->free) {
            used_size += current->size + sizeof(memory_block_t);
        }
        
        if (current->next == NULL) break;
        current = current->next;
    }
    
    return used_size;
}

// Get total memory size
size_t memory_get_total(void) {
    return MEMORY_POOL_SIZE;
}

// Debug: dump memory blocks
void memory_dump(void) {
    shell_print_string("Memory dump:\n");
    memory_block_t* current = (memory_block_t*)memory_pool;
    int block_num = 0;
    
    while ((unsigned char*)current < memory_pool + MEMORY_POOL_SIZE) {
        char buffer[64];
        shell_print_string("Block ");
        itoa(block_num++, buffer);
        shell_print_string(buffer);
        shell_print_string(": size=");
        itoa(current->size, buffer);
        shell_print_string(buffer);
        shell_print_string(" free=");
        shell_print_string(current->free ? "yes" : "no");
        shell_print_string("\n");
        
        if (current->next == NULL) break;
        current = current->next;
    }
}

// Debug: check memory integrity
int memory_check_integrity(void) {
    memory_block_t* current = (memory_block_t*)memory_pool;
    
    while ((unsigned char*)current < memory_pool + MEMORY_POOL_SIZE) {
        if ((unsigned char*)current + sizeof(memory_block_t) + current->size > memory_pool + MEMORY_POOL_SIZE) {
            return 0; // Invalid block
        }
        
        if (current->next == NULL) break;
        current = current->next;
    }
    
    return 1; // Memory is valid
}