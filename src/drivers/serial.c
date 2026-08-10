#include "serial.h"
#include "io.h"   

#define COM1 0x3F8


static int tx_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

int serial_init(void) {
    outb(COM1 + 1, 0x00); 
    outb(COM1 + 3, 0x80);  
    outb(COM1 + 0, 0x03);  
    outb(COM1 + 1, 0x00);  
    outb(COM1 + 3, 0x03); 
    outb(COM1 + 2, 0xC7); 
    outb(COM1 + 4, 0x0B);  
 

    outb(COM1 + 4, 0x1E);  
    outb(COM1 + 0, 0xAE); 
    if (inb(COM1 + 0) != 0xAE)
        return 1;          

    outb(COM1 + 4, 0x0F);  
    return 0;
}

void serial_putc(char c) {
    if (c == '\n') {          
        while (!tx_empty()) {}
        outb(COM1, '\r');
    }
    while (!tx_empty()) {}
    outb(COM1, (uint8_t)c);
}

void serial_write(const char* s) {
    for (; *s; s++)
        serial_putc(*s);
}

void serial_write_hex(uint64_t v) {
    static const char d[] = "0123456789ABCDEF";
    serial_write("0x");
    for (int i = 60; i >= 0; i -= 4)
        serial_putc(d[(v >> i) & 0xF]);
}