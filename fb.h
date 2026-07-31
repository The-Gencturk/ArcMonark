#include <stdint.h>

int  fb_init(void);
void clear_screen(void);
void putchar(char c);
void print(const char* s);
void set_color(uint32_t fg, uint32_t bg);
void color(uint32_t fd);