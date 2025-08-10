#include <stddef.h>
#include "string_utils.h"

// Custom string functions to avoid conflicts with builtin functions
size_t my_strlen(const char* s) {
    size_t i = 0;
    while (s[i]) i++;
    return i;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* my_strncpy(char* dest, const char* src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

int my_strncmp(const char* a, const char* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i] || !a[i] || !b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
    }
    return 0;
}

char* strcat(char* dest, const char* src) {
    char* d = dest + my_strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}

size_t my_strspn(const char* s, const char* accept) {
    size_t count = 0;
    while (*s) {
        const char* a = accept;
        int found = 0;
        while (*a) {
            if (*s == *a) { found = 1; break; }
            a++;
        }
        if (!found) break;
        count++;
        s++;
    }
    return count;
}

size_t my_strcspn(const char* s, const char* reject) {
    int count = 0;
    while (*s) {
        const char* r = reject;
        int found = 0;
        while (*r) {
            if (*s == *r) { found = 1; break; }
            r++;
        }
        if (found) break;
        count++;
        s++;
    }
    return count;
}

char* strtok_r(char* str, const char* delim, char** saveptr) {
    char* token;
    if (str == NULL) str = *saveptr;
    str += my_strspn(str, delim);
    if (*str == '\0') {
        *saveptr = str;
        return NULL;
    }
    token = str;
    str = str + my_strcspn(str, delim);
    if (*str != '\0') {
        *str = '\0';
        str++;
    }
    *saveptr = str;
    return token;
}

char* itoa(int value, char* str) {
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    
    int is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    int i = 0;
    while (value != 0) {
        str[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    
    return str;
}

void* memset(void* s, int c, size_t n) {
    unsigned char* ptr = (unsigned char*)s;
    unsigned char value = (unsigned char)c;
    
    for (size_t i = 0; i < n; i++) {
        ptr[i] = value;
    }
    
    return s;
}