#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

// Memory management for SkyOS
#define MEMORY_POOL_SIZE (1024 * 1024) // 1MB memory pool
#define MIN_BLOCK_SIZE 16
#define MAX_BLOCK_SIZE (MEMORY_POOL_SIZE / 4)

// Memory block structure
typedef struct memory_block {
    size_t size;
    int free;
    struct memory_block* next;
    struct memory_block* prev;
} memory_block_t;

// Memory management functions
void memory_init(void);
void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t new_size);

// Memory statistics
size_t memory_get_free(void);
size_t memory_get_used(void);
size_t memory_get_total(void);

// Debug functions
void memory_dump(void);
int memory_check_integrity(void);

#endif // MEMORY_H