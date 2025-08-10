#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

// Custom string functions to avoid conflicts with builtin functions
#define strlen my_strlen
#define strncmp my_strncmp
#define strncpy my_strncpy
#define strspn my_strspn
#define strcspn my_strcspn

// Safe string operation macros
#define SAFE_STRCPY(dest, src, max_len) safe_strcpy(dest, src, max_len)
#define SAFE_STRCAT(dest, src, max_len) safe_strcat(dest, src, max_len)

// Function declarations
size_t my_strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* my_strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* a, const char* b);
int my_strncmp(const char* a, const char* b, size_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* s, int c);
size_t my_strspn(const char* s, const char* accept);
size_t my_strcspn(const char* s, const char* reject);
char* strtok_r(char* str, const char* delim, char** saveptr);
char* itoa(int value, char* str);
void* memset(void* s, int c, size_t n);

// Safe string functions
char* safe_strcpy(char* dest, const char* src, size_t dest_size);
char* safe_strcat(char* dest, const char* src, size_t dest_size);

#endif // STRING_UTILS_H