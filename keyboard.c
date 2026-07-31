#include "keyboard.h"
#include "io.h"
#include "vga.h"

static const char scancode_ascii[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'','`', 0,  '\\','z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 0,   '*', 0,   ' ',
};

char keyboard_getchar(void) {
    while (1) {
        if (inb(0x64) & 1) {              // output buffer dolu mu?
            unsigned char sc = inb(0x60);
            if (sc & 0x80) continue;      // tus birakma → gormezden gel
            char c = scancode_ascii[sc];
            if (c) return c;
        }
    }
}

void read_line(char* buf, int max) {
    int len = 0;
    while (1) {
        char c = keyboard_getchar();
        if (c == '\n') { putchar('\n'); buf[len] = '\0'; return; }
        else if (c == '\b') { if (len > 0) { len--; putchar('\b'); } }
        else if (len < max - 1) { buf[len++] = c; putchar(c); }
    }
}