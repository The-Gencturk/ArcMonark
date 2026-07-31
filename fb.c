#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "fb.h"
#include "font8x16.h"   


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


static uint32_t* fb;         
static uint64_t  pitch;      
static uint32_t  width, height;
static uint32_t  cur_x, cur_y;   
static uint32_t  fg = 0xFFFFFFFF, bg = 0x00000000;

#define CH_W 8
#define CH_H 16

int fb_init(void) {
    if (!fb_request.response || fb_request.response->framebuffer_count < 1)
        return 0;
    struct limine_framebuffer* f = fb_request.response->framebuffers[0];
    fb = (uint32_t*)f->address;
    pitch = f->pitch;
    width = f->width;
    height = f->height;
    cur_x = cur_y = 0;
    return 1;
}

static inline void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= width || y >= height) return;
    fb[y * (pitch / 4) + x] = color;
}

void clear_screen(void) {
    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++)
            fb[y * (pitch / 4) + x] = bg;
    cur_x = cur_y = 0;
}


static void draw_char(char c, uint32_t px, uint32_t py) {
    const uint8_t* glyph = font8x16[(uint8_t)c];
    for (int row = 0; row < CH_H; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < CH_W; col++)
            put_pixel(px + col, py + row, (bits & (0x80 >> col)) ? fg : bg);
    }
}

void draw_cursor(void) {
    draw_char('_', cur_x * CH_W, cur_y * CH_H);
}

void clear_cursor(void) {
    draw_char(' ', cur_x * CH_W, cur_y * CH_H);
}


static void scroll(void) {
    uint32_t line_px = CH_H * (pitch / 4);
    uint32_t total = (pitch / 4) * height;
    for (uint32_t i = 0; i + line_px < total; i++)
        fb[i] = fb[i + line_px];
    for (uint32_t i = total - line_px; i < total; i++)
        fb[i] = bg;
    cur_y--;
}

void putchar(char c) {
    clear_cursor();
    if (c == '\n') { cur_x = 0; cur_y++; }
    else if (c == '\r') { cur_x = 0; }
    else if (c == '\b') { if (cur_x) cur_x--; draw_char(' ', cur_x * CH_W, cur_y * CH_H); }
    else {
        draw_char(c, cur_x * CH_W, cur_y * CH_H);
        if (++cur_x >= width / CH_W) { cur_x = 0; cur_y++; }
    }
    while (cur_y >= height / CH_H) scroll();
    draw_cursor();
}

void print(const char* s) { while (*s) putchar(*s++); }

void color(uint32_t new_fg) { fg = new_fg; }

void set_color(uint32_t new_fg, uint32_t new_bg) { fg = new_fg; bg = new_bg; }

