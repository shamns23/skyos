#include "io.h"

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

void outw(unsigned short port, unsigned short data) {
    __asm__ volatile ("outw %0, %1" : : "a"(data), "Nd"(port));
}

void outl(unsigned short port, unsigned int data) {
    __asm__ volatile ("outl %0, %1" : : "a"(data), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

unsigned short inw(unsigned short port) {
    unsigned short result;
    __asm__ volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

unsigned int inl(unsigned short port) {
    unsigned int result;
    __asm__ volatile ("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void delay() {
    for (volatile int i = 0; i < 1000000; i++);
}