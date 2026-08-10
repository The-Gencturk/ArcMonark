#include "gfx.h"
#include "heap.h"   
#include "string.h"   

extern const uint8_t font8x16[128][16];

static uint8_t* g_hw;     
static uint8_t* g_fb;     
static uint32_t g_width;
static uint32_t g_height;
static uint32_t g_pitch;    
static uint16_t g_bpp;

void gfx_init(void* framebuffer, uint32_t width, uint32_t height,
    uint32_t pitch, uint16_t bpp) {
    g_hw = (uint8_t*)framebuffer;
    g_fb = g_hw;            
    g_width = width;
    g_height = height;
    g_pitch = pitch;
    g_bpp = bpp;
}


int gfx_backbuffer_init(void) {
    size_t bytes = (size_t)g_height * g_pitch;
    uint8_t* buf = (uint8_t*)kmalloc(bytes);
    if (!buf) return -1;
    memset(buf, 0, bytes);
    g_fb = buf;
    return 0;
}


void gfx_present(void) {
    if (g_fb == g_hw) return;                 
    size_t n = ((size_t)g_height * g_pitch) / 4;
    const uint32_t* s = (const uint32_t*)g_fb;
    uint32_t* d = (uint32_t*)g_hw;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
}

uint32_t gfx_width(void) { return g_width; }
uint32_t gfx_height(void) { return g_height; }

void gfx_pixel(uint32_t x, uint32_t y, uint32_t argb) {
    if (x >= g_width || y >= g_height) return;     
    uint8_t* row = g_fb + (size_t)y * g_pitch;
    ((uint32_t*)row)[x] = argb;
}

void gfx_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb) {
    if (x >= g_width || y >= g_height) return;
    if (x + w > g_width)  w = g_width - x;         
    if (y + h > g_height) h = g_height - y;         
    for (uint32_t j = 0; j < h; j++) {
        uint32_t* row = (uint32_t*)(g_fb + (size_t)(y + j) * g_pitch) + x;
        for (uint32_t i = 0; i < w; i++)
            row[i] = argb;
    }
}

void gfx_hline(uint32_t x, uint32_t y, uint32_t len, uint32_t argb) {
    gfx_fill_rect(x, y, len, 1, argb);
}

void gfx_vline(uint32_t x, uint32_t y, uint32_t len, uint32_t argb) {
    gfx_fill_rect(x, y, 1, len, argb);
}

void gfx_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb) {
    if (w == 0 || h == 0) return;
    gfx_hline(x, y, w, argb);   
    gfx_hline(x, y + h - 1, w, argb);   
    gfx_vline(x, y, h, argb);   
    gfx_vline(x + w - 1, y, h, argb);   
}

void gfx_clear(uint32_t argb) {
    gfx_fill_rect(0, 0, g_width, g_height, argb);
}

void gfx_blit(const uint32_t* src, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    if (x >= g_width || y >= g_height) return;
    uint32_t cw = (x + w > g_width) ? g_width - x : w;
    uint32_t ch = (y + h > g_height) ? g_height - y : h;
    for (uint32_t j = 0; j < ch; j++) {
        uint32_t* dst = (uint32_t*)(g_fb + (size_t)(y + j) * g_pitch) + x;
        const uint32_t* s = src + (size_t)j * w;   
        for (uint32_t i = 0; i < cw; i++)
            dst[i] = s[i];
    }
}

void gfx_char(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) uc = '?';                       
    const uint8_t* glyph = font8x16[uc];
    for (uint32_t row = 0; row < 16; row++) {
        uint8_t bits = glyph[row];
        for (uint32_t col = 0; col < 8; col++) {
            uint32_t color = (bits & (0x80 >> col)) ? fg : bg;
            gfx_pixel(x + col, y + row, color);
        }
    }
}

void gfx_text(const char* s, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
    for (; *s; s++) {
        gfx_char(*s, x, y, fg, bg);
        x += 8;
        if (x + 8 > g_width) { x = 0; y += 16; }   
    }
}

//void gfx_