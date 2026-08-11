#include "mouse.h"
#include "io.h"
#include "idt.h"
#include "pic.h"
#include "event.h"

// 8042 PS/2 denetleyici portlari
#define PS2_DATA    0x60
#define PS2_STATUS  0x64   // okuma: durum register'i
#define PS2_CMD     0x64   // yazma: komut register'i

// Durum register'i bitleri
#define ST_OUT_FULL 0x01   // cikis tamponu dolu (0x60'dan okunabilir)
#define ST_IN_FULL  0x02   // giris tamponu dolu (yazmadan once 0 olmali)
#define ST_AUX_DATA 0x20   // veri aux cihazindan (fare)

static int      s_x, s_y;              // imleç konumu (piksel)
static uint32_t s_w = 640, s_h = 480;  // ekran siniri
static uint8_t  s_buttons;             // guncel tus durumu

static uint8_t  packet[3];             // 3 baytlik PS/2 paketi
static int      cycle;                 // paket icindeki bayt sayaci

// --- 8042 ile konusma yardimcilari (polling) --------------------------------
static void ps2_wait_write(void) {     // giris tamponu bosalsin
    for (int i = 0; i < 100000; i++)
        if ((inb(PS2_STATUS) & ST_IN_FULL) == 0) return;
}
static void ps2_wait_read(void) {      // cikis tamponu dolsun
    for (int i = 0; i < 100000; i++)
        if (inb(PS2_STATUS) & ST_OUT_FULL) return;
}

static void mouse_cmd(uint8_t cmd) {   // fareye komut (once 0xD4 oneki)
    ps2_wait_write(); outb(PS2_CMD, 0xD4);
    ps2_wait_write(); outb(PS2_DATA, cmd);
}
static uint8_t mouse_read(void) {
    ps2_wait_read(); return inb(PS2_DATA);
}

// --- IRQ12 kesme isleyicisi (vektor 0x2C) -----------------------------------
__attribute__((interrupt))
static void mouse_irq(void* frame) {
    (void)frame;

    // Yalniz fare (aux) verisi geldiyse oku; degilse (klavye) dokunma.
    if (inb(PS2_STATUS) & ST_AUX_DATA) {
        uint8_t data = inb(PS2_DATA);
        switch (cycle) {
        case 0:
            if (!(data & 0x08)) break;   // bit3 daima 1 olmali; degilse senkron kaybi -> at
            packet[0] = data; cycle = 1; break;
        case 1:
            packet[1] = data; cycle = 2; break;
        case 2: {
            packet[2] = data; cycle = 0;
            if (packet[0] & 0xC0) break;             // X/Y tasmasi -> paketi at
            // 9-bit isaretli hareket: isaret biti flag baytinda.
            int dx = (int)packet[1] - ((packet[0] << 4) & 0x100);
            int dy = (int)packet[2] - ((packet[0] << 3) & 0x100);
            int oldx = s_x, oldy = s_y;
            s_x += dx;
            s_y -= dy;                               // ekran Y'si ters (yukari = kucuk y)
            if (s_x < 0) s_x = 0;
            if (s_y < 0) s_y = 0;
            if (s_x >= (int)s_w) s_x = (int)s_w - 1;
            if (s_y >= (int)s_h) s_y = (int)s_h - 1;

            uint8_t new_btn = packet[0] & 0x07;

            // Hareket olayi (ekran-uzayi bagil delta, clamp sonrasi).
            int edx = s_x - oldx, edy = s_y - oldy;
            if (edx || edy) {
                event_t e = {0};
                e.type = EV_MOUSE_MOVE;
                e.x = (int16_t)s_x; e.y = (int16_t)s_y;
                e.dx = (int16_t)edx; e.dy = (int16_t)edy;
                e.buttons = new_btn;
                event_push(&e);
            }

            // Tus degisim olaylari (sol/sag/orta ayri ayri).
            uint8_t changed = new_btn ^ s_buttons;
            for (uint8_t bit = 0x01; bit <= 0x04; bit <<= 1) {
                if (!(changed & bit)) continue;
                event_t e = {0};
                e.type    = (new_btn & bit) ? EV_MOUSE_DOWN : EV_MOUSE_UP;
                e.button  = bit;
                e.buttons = new_btn;
                e.x = (int16_t)s_x; e.y = (int16_t)s_y;
                event_push(&e);
            }

            s_buttons = new_btn;
            break;
        }
        }
    }

    pic_send_eoi(12);   // IRQ>=8 -> her iki PIC'e EOI
}

void mouse_init(uint32_t screen_w, uint32_t screen_h) {
    s_w = screen_w; s_h = screen_h;
    s_x = (int)screen_w / 2;
    s_y = (int)screen_h / 2;
    cycle = 0; s_buttons = 0;

    // Bekleyen baytlari temizle
    while (inb(PS2_STATUS) & ST_OUT_FULL) (void)inb(PS2_DATA);

    // Aux (fare) cihazini etkinlestir
    ps2_wait_write(); outb(PS2_CMD, 0xA8);

    // Komut baytini oku -> IRQ12'yi ac (bit1), fare saatini ac (bit5=0) -> geri yaz
    ps2_wait_write(); outb(PS2_CMD, 0x20);
    uint8_t status = mouse_read();
    status |=  0x02;    // IRQ12 aktif
    status &= ~0x20;    // fare saati aktif
    ps2_wait_write(); outb(PS2_CMD, 0x60);
    ps2_wait_write(); outb(PS2_DATA, status);

    // Fareye: varsayilanlar + veri raporlamayi ac (her biri 0xFA ACK doner)
    mouse_cmd(0xF6); (void)mouse_read();
    mouse_cmd(0xF4); (void)mouse_read();

    // IRQ12 -> vektor 0x2C; kaskad (IRQ2) ve IRQ12 maskesini ac
    idt_set_gate(0x2C, mouse_irq, IDT_INT_GATE);
    irq_clear_mask(2);
    irq_clear_mask(12);
}

int     mouse_get_x(void)       { return s_x; }
int     mouse_get_y(void)       { return s_y; }
uint8_t mouse_get_buttons(void) { return s_buttons; }
