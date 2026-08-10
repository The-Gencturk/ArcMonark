#include "keyboard.h"
#include "io.h"
#include "fb.h"
#include "idt.h"
#include "pic.h"
#include <stdint.h>

static const char scancode_ascii[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'','`', 0,  '\\','z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 0,   '*', 0,   ' ',
};

static const char scancode_ascii_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+', '\b','\t','Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M', '<', '>', '?', 0,   '*', 0,   ' ',
};

#define SC_LSHIFT_DOWN   0x2A
#define SC_RSHIFT_DOWN   0x36
#define SC_LSHIFT_UP     0xAA
#define SC_RSHIFT_UP     0xB6
#define SC_CAPS_DOWN     0x3A

static volatile int shift_down = 0;  
static volatile int caps_lock = 0;   

#define KBD_BUF_SIZE 256
static volatile char     kbd_buf[KBD_BUF_SIZE];
static volatile uint32_t kbd_head = 0;   
static volatile uint32_t kbd_tail = 0;   

static inline int is_letter(char c) {
    return (c >= 'a' && c <= 'z');
}

__attribute__((interrupt))
void keyboard_handler(void* frame) {
    (void)frame;

    unsigned char sc = inb(0x60);

    switch (sc) {
    case SC_LSHIFT_DOWN:
    case SC_RSHIFT_DOWN: shift_down = 1; goto eoi;
    case SC_LSHIFT_UP:
    case SC_RSHIFT_UP:   shift_down = 0; goto eoi;
    case SC_CAPS_DOWN:   caps_lock ^= 1; goto eoi;  // toggle
    }

    if (sc & 0x80) goto eoi;

    char c = shift_down ? scancode_ascii_shift[sc] : scancode_ascii[sc];
    if (!c) goto eoi;

    if (caps_lock && is_letter(scancode_ascii[sc])) {
        int upper = caps_lock ^ shift_down;
        c = upper ? scancode_ascii_shift[sc] : scancode_ascii[sc];
    }


    uint32_t next = (kbd_head + 1) % KBD_BUF_SIZE;
    if (next != kbd_tail) {
        kbd_buf[kbd_head] = c;
        kbd_head = next;
    }

eoi:
    outb(0x20, 0x20);   
}

char keyboard_getchar(void) {
    while (kbd_head == kbd_tail) {
        __asm__ volatile ("hlt");
    }
    char c = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
    return c;
}

// Non-bloklayici: tampon icinde bekleyen tus var mi?
int keyboard_haskey(void) {
    return kbd_head != kbd_tail;
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

void keyboard_init(void) {
    idt_set_gate(0x21, keyboard_handler, IDT_INT_GATE);

    while (inb(0x64) & 1) {
        inb(0x60);
    }

    irq_clear_mask(1);   
}