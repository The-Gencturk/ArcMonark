// Metin konsolu. Artik framebuffer'a DOGRUDAN yazmaz; tum cizim gfx arka
// tamponu + present uzerinden gider (tek cizim yolu). Framebuffer sahibi
// gfx.c'dir (bkz. gfx_init).
#include "fb.h"
#include "gfx.h"

#define CH_W 8
#define CH_H 16

static uint32_t cur_x, cur_y;                          // hucre (kolon, satir)
static uint32_t con_fg = 0xFFFFFFFF, con_bg = 0xFF000000;

void set_color(uint32_t fg, uint32_t bg) { con_fg = fg; con_bg = bg; }
void color(uint32_t fg)                  { con_fg = fg; }

void clear_screen(void) {
    gfx_clear(con_bg);
    gfx_present();
    cur_x = cur_y = 0;
}

static void con_scroll(void) {
    gfx_scroll_up(CH_H, con_bg);
    gfx_present();               // kaydirma seyrek -> tam present kabul edilebilir
    cur_y--;
}

void putchar(char c) {
    if (c == '\n')      { cur_x = 0; cur_y++; }
    else if (c == '\r') { cur_x = 0; }
    else if (c == '\b') {
        if (cur_x) cur_x--;
        gfx_fill_rect(cur_x * CH_W, cur_y * CH_H, CH_W, CH_H, con_bg);
        gfx_present_rect(cur_x * CH_W, cur_y * CH_H, CH_W, CH_H);
    } else {
        gfx_char(c, cur_x * CH_W, cur_y * CH_H, con_fg, con_bg);
        gfx_present_rect(cur_x * CH_W, cur_y * CH_H, CH_W, CH_H);
        if (++cur_x >= gfx_width() / CH_W) { cur_x = 0; cur_y++; }
    }
    while (cur_y >= gfx_height() / CH_H) con_scroll();
}

void print(const char* s) { while (*s) putchar(*s++); }
