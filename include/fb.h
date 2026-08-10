#include <stdint.h>

// Metin konsolu. Framebuffer'i gfx katmani yonetir (bkz. gfx.h); bu modul
// yalnizca hucre tabanli metin cizimini gfx primitifleri uzerinden yapar.
void clear_screen(void);
void putchar(char c);
void print(const char* s);
void set_color(uint32_t fg, uint32_t bg);
void color(uint32_t fg);
