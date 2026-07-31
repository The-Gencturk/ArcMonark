#include "vga.h"
#include "config.h"
#include "io.h"

#define VGA_MEMORY  ((volatile unsigned short*)0xB8000)
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

static int cursor_row = 0;
static int cursor_col = 0;
static unsigned char text_color;

static unsigned char make_color(unsigned char fg, unsigned char bg) {
    return fg | (bg << 4);
}
static unsigned short make_entry(char c, unsigned char color) {
    return (unsigned short)c | ((unsigned short)color << 8);
}

void set_color(unsigned char fg, unsigned char bg) {
    text_color = make_color(fg, bg);
}

void vga_init(void) {
    set_color(THEME_FG, THEME_BG);   // renk artik config.h'ten geliyor
    clear_screen();
}

static void update_cursor(void) {
    unsigned short pos = cursor_row * VGA_WIDTH + cursor_col;
    outb(0x3D4, 14); outb(0x3D5, pos >> 8);
    outb(0x3D4, 15); outb(0x3D5, pos & 0xFF);
}

static void scroll(void) {
    for (int r = 1; r < VGA_HEIGHT; r++)
        for (int c = 0; c < VGA_WIDTH; c++)
            VGA_MEMORY[(r - 1) * VGA_WIDTH + c] = VGA_MEMORY[r * VGA_WIDTH + c];
    unsigned short blank = make_entry(' ', text_color);
    for (int c = 0; c < VGA_WIDTH; c++)
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + c] = blank;
}

void clear_screen(void) {
    unsigned short blank = make_entry(' ', text_color);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA_MEMORY[i] = blank;
    cursor_row = 0;
    cursor_col = 0;
    update_cursor();
}

void putchar(char c) {
    if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = make_entry(' ', text_color);
        }
        update_cursor();
        return;
    }
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    }
    else {
        VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = make_entry(c, text_color);
        if (++cursor_col >= VGA_WIDTH) { cursor_col = 0; cursor_row++; }
    }
    if (cursor_row >= VGA_HEIGHT) { scroll(); cursor_row = VGA_HEIGHT - 1; }
    update_cursor();
}

void print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) putchar(str[i]);
}

void print_hex(unsigned int n) {
    const char* d = "0123456789ABCDEF";
    print("0x");
    for (int i = 28; i >= 0; i -= 4) putchar(d[(n >> i) & 0xF]);
}

void print_dec(unsigned int n) {
    char buf[11];
    int i = 10;
    buf[i--] = '\0';
    if (n == 0) { putchar('0'); return; }
    while (n > 0) { buf[i--] = '0' + (n % 10); n /= 10; }
    print(&buf[i + 1]);
}