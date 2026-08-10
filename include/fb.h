#include <stdint.h>

int  fb_init(void);
void clear_screen(void);
void putchar(char c);
void print(const char* s);
void set_color(uint32_t fg, uint32_t bg);
void color(uint32_t fd);


void*    fb_address(void);
uint32_t fb_width(void);
uint32_t fb_height(void);
uint32_t fb_pitch(void);   
uint16_t fb_bpp(void);