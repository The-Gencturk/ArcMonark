#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

// Cekirdek genelinde ortak girdi olay kuyrugu. Klavye/fare kesme isleyicileri
// olay uretir (event_push), GUI olay dongusu tuketir (event_poll). Boylece
// gorsel katman tek bir kaynaktan hem klavye hem fare olaylarini alir.

typedef enum {
    EV_NONE = 0,
    EV_KEY_DOWN,      // tusa basildi   (ch + scancode)
    EV_KEY_UP,        // tus birakildi  (scancode)
    EV_MOUSE_MOVE,    // fare hareketi  (x,y,dx,dy,buttons)
    EV_MOUSE_DOWN,    // tusa basildi   (button = degisen tus maskesi)
    EV_MOUSE_UP,      // tus birakildi  (button = degisen tus maskesi)
} event_type_t;

typedef struct {
    uint8_t  type;        // event_type_t
    char     ch;          // ASCII (yoksa 0)   -- EV_KEY_*
    uint8_t  scancode;    // ham tarama kodu   -- EV_KEY_*
    uint8_t  buttons;     // guncel fare tus maskesi
    uint8_t  button;      // degisen tus       -- EV_MOUSE_DOWN/UP
    int16_t  x, y;        // mutlak imleç konumu
    int16_t  dx, dy;      // bagil hareket     -- EV_MOUSE_MOVE
} event_t;

void event_init(void);
void event_push(const event_t* e);   // ISR baglaminden cagrilir
int  event_poll(event_t* out);       // olay varsa 1 dondurur ve *out doldurur
int  event_pending(void);            // bekleyen olay var mi (non-bloklayici)

#endif
