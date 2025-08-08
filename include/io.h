#ifndef IO_H
#define IO_H

// I/O port functions
void outb(unsigned short port, unsigned char data);
void outw(unsigned short port, unsigned short data);
unsigned char inb(unsigned short port);
void delay();

#endif // IO_H