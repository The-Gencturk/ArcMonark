#pragma once
#include <stdint.h>
#include <stddef.h>


#define GFX_RGB(r, g, b) \
    (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

void     gfx_init(void* framebuffer, uint32_t width, uint32_t height,
    uint32_t pitch, uint16_t bpp);
uint32_t gfx_width(void);
uint32_t gfx_height(void);


int  gfx_backbuffer_init(void);   
void gfx_present(void);          

void gfx_clear(uint32_t argb);
void gfx_pixel(uint32_t x, uint32_t y, uint32_t argb);
void gfx_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb);
void gfx_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb); // çerçeve
void gfx_hline(uint32_t x, uint32_t y, uint32_t len, uint32_t argb);
void gfx_vline(uint32_t x, uint32_t y, uint32_t len, uint32_t argb);
void gfx_blit(const uint32_t* src, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void gfx_char(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void gfx_text(const char* s, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);