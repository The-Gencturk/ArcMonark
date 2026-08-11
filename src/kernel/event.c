#include "event.h"
#include "irqlock.h"

// Tek yazici (ISR) - tek okuyucu (task) halka tampon. Tampon dolarsa en eski
// olay dusurulur (girdi kaybi, cokme yerine). Boyut 2'nin kuvveti olmali.
#define EVQ_SIZE 256
#define EVQ_MASK (EVQ_SIZE - 1)

static event_t          q[EVQ_SIZE];
static volatile uint32_t q_head;   // yazma konumu (ISR)
static volatile uint32_t q_tail;   // okuma konumu (task)

void event_init(void) {
    q_head = 0;
    q_tail = 0;
}

void event_push(const event_t* e) {
    uint32_t next = (q_head + 1) & EVQ_MASK;
    if (next == q_tail) {
        // Tampon dolu: en eski olayi dusur ki en yeni olay kaybolmasin.
        q_tail = (q_tail + 1) & EVQ_MASK;
    }
    q[q_head] = *e;
    q_head = next;
}

int event_poll(event_t* out) {
    uint64_t f = irq_save();
    if (q_tail == q_head) { irq_restore(f); return 0; }
    *out = q[q_tail];
    q_tail = (q_tail + 1) & EVQ_MASK;
    irq_restore(f);
    return 1;
}

int event_pending(void) {
    return q_head != q_tail;
}
