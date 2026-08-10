#pragma once
#include <stdint.h>
#include <stddef.h>

// 0xAARRGGBB; alfa 0xFF (framebuffer XRGB, alfa yok sayilir ama tek sema).
#define GFX_RGB(r, g, b) \
    (0xFF000000u | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

// Framebuffer sahibi gfx katmanidir. gfx_init Limine framebuffer'ini sorgular.
int      gfx_init(void);          // 1 = tamam, 0 = framebuffer yok
uint32_t gfx_width(void);
uint32_t gfx_height(void);

// Cift tamponlama
int  gfx_backbuffer_init(void);   // ekran boyutunda arka tampon ayir (0=ok, -1=bellek yok)
void gfx_present(void);           // arka tampon -> ekran (tam)
void gfx_present_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);   // kismi present
void gfx_copy_out(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t* dst); // arka tampondan oku
void gfx_scroll_up(uint32_t dy, uint32_t argb);   // arka tamponu dy piksel yukari kaydir

// Cizim primitifleri (aktif hedefe: arka tampon varsa oraya, yoksa ekrana)
void gfx_clear(uint32_t argb);
void gfx_pixel(uint32_t x, uint32_t y, uint32_t argb);
void gfx_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb);
void gfx_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb); // cerceve
void gfx_hline(uint32_t x, uint32_t y, uint32_t len, uint32_t argb);
void gfx_vline(uint32_t x, uint32_t y, uint32_t len, uint32_t argb);
void gfx_blit(const uint32_t* src, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void gfx_char(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void gfx_text(const char* s, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
