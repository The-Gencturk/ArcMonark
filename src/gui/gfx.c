#include "gfx.h"
#include "heap.h"      // kmalloc (arka tampon)
#include "string.h"    // memset
#include <limine.h>
#include "font8x16.h"  // tek metin cizim yeri artik burasi

// --- Limine framebuffer istegi (cekirdek tarafi) ---------------------------
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[3] = LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start[4] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end[2] = LIMINE_REQUESTS_END_MARKER;

// --- durum -----------------------------------------------------------------
static uint8_t* g_hw;       // gercek donanim framebuffer'i (present hedefi)
static uint8_t* g_fb;       // aktif cizim hedefi (cift tampon aciksa = arka tampon)
static uint32_t g_width;
static uint32_t g_height;
static uint32_t g_pitch;    // BAYT cinsinden
static uint16_t g_bpp;

// Limine framebuffer'ini sorgular ve gfx'i hazirlar. _start bunu ilk cagirir.
int gfx_init(void) {
    if (!fb_request.response || fb_request.response->framebuffer_count < 1)
        return 0;
    struct limine_framebuffer* f = fb_request.response->framebuffers[0];
    g_hw     = (uint8_t*)f->address;
    g_fb     = g_hw;             // varsayilan: dogrudan ekrana (tek tampon)
    g_width  = f->width;
    g_height = f->height;
    g_pitch  = f->pitch;
    g_bpp    = f->bpp;
    return 1;
}

// --- cift tamponlama --------------------------------------------------------
int gfx_backbuffer_init(void) {
    size_t bytes = (size_t)g_height * g_pitch;
    uint8_t* buf = (uint8_t*)kmalloc(bytes);
    if (!buf) return -1;
    memset(buf, 0, bytes);
    g_fb = buf;                 // primitifler artik arka tampona yazar
    return 0;
}

void gfx_present(void) {
    if (g_fb == g_hw) return;
    size_t n = ((size_t)g_height * g_pitch) / 4;
    const uint32_t* s = (const uint32_t*)g_fb;
    uint32_t* d = (uint32_t*)g_hw;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
}

// Arka tamponun yalniz bir dikdortgenini ekrana kopyalar (QEMU'da cok daha hizli).
void gfx_present_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    if (g_fb == g_hw) return;
    if (x >= g_width || y >= g_height) return;
    if (x + w > g_width)  w = g_width - x;
    if (y + h > g_height) h = g_height - y;
    for (uint32_t j = 0; j < h; j++) {
        size_t off = (size_t)(y + j) * g_pitch + (size_t)x * 4;
        const uint32_t* s = (const uint32_t*)(g_fb + off);
        uint32_t* d = (uint32_t*)(g_hw + off);
        for (uint32_t i = 0; i < w; i++) d[i] = s[i];
    }
}

// Arka tampondan siki (w-adimli) bir dikdortgeni dst'ye okur (gfx_blit'in tersi).
void gfx_copy_out(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t* dst) {
    for (uint32_t j = 0; j < h; j++) {
        const uint32_t* s = (const uint32_t*)(g_fb + (size_t)(y + j) * g_pitch) + x;
        uint32_t* d = dst + (size_t)j * w;
        for (uint32_t i = 0; i < w; i++) d[i] = s[i];
    }
}

// Arka tamponu dy piksel yukari kaydirir, acilan alt kismi argb ile doldurur.
void gfx_scroll_up(uint32_t dy, uint32_t argb) {
    if (dy == 0) return;
    if (dy >= g_height) { gfx_clear(argb); return; }
    for (uint32_t y = 0; y < g_height - dy; y++) {
        uint32_t*       dst = (uint32_t*)(g_fb + (size_t)y * g_pitch);
        const uint32_t* src = (const uint32_t*)(g_fb + (size_t)(y + dy) * g_pitch);
        for (uint32_t x = 0; x < g_width; x++) dst[x] = src[x];
    }
    gfx_fill_rect(0, g_height - dy, g_width, dy, argb);
}

uint32_t gfx_width(void)  { return g_width; }
uint32_t gfx_height(void) { return g_height; }

// --- cizim primitifleri -----------------------------------------------------
void gfx_pixel(uint32_t x, uint32_t y, uint32_t argb) {
    if (x >= g_width || y >= g_height) return;
    ((uint32_t*)(g_fb + (size_t)y * g_pitch))[x] = argb;
}

void gfx_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb) {
    if (x >= g_width || y >= g_height) return;
    if (x + w > g_width)  w = g_width - x;
    if (y + h > g_height) h = g_height - y;
    for (uint32_t j = 0; j < h; j++) {
        uint32_t* row = (uint32_t*)(g_fb + (size_t)(y + j) * g_pitch) + x;
        for (uint32_t i = 0; i < w; i++) row[i] = argb;
    }
}

void gfx_hline(uint32_t x, uint32_t y, uint32_t len, uint32_t argb) { gfx_fill_rect(x, y, len, 1, argb); }
void gfx_vline(uint32_t x, uint32_t y, uint32_t len, uint32_t argb) { gfx_fill_rect(x, y, 1, len, argb); }

void gfx_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t argb) {
    if (w == 0 || h == 0) return;
    gfx_hline(x, y,         w, argb);
    gfx_hline(x, y + h - 1, w, argb);
    gfx_vline(x,         y, h, argb);
    gfx_vline(x + w - 1, y, h, argb);
}

void gfx_clear(uint32_t argb) { gfx_fill_rect(0, 0, g_width, g_height, argb); }

void gfx_blit(const uint32_t* src, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    if (x >= g_width || y >= g_height) return;
    uint32_t cw = (x + w > g_width)  ? g_width  - x : w;
    uint32_t ch = (y + h > g_height) ? g_height - y : h;
    for (uint32_t j = 0; j < ch; j++) {
        uint32_t* dst = (uint32_t*)(g_fb + (size_t)(y + j) * g_pitch) + x;
        const uint32_t* s = src + (size_t)j * w;
        for (uint32_t i = 0; i < cw; i++) dst[i] = s[i];
    }
}

void gfx_char(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) uc = '?';
    const uint8_t* glyph = font8x16[uc];
    for (uint32_t row = 0; row < 16; row++) {
        uint8_t bits = glyph[row];
        for (uint32_t col = 0; col < 8; col++)
            gfx_pixel(x + col, y + row, (bits & (0x80 >> col)) ? fg : bg);
    }
}

void gfx_text(const char* s, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
    for (; *s; s++) {
        gfx_char(*s, x, y, fg, bg);
        x += 8;
        if (x + 8 > g_width) { x = 0; y += 16; }
    }
}
