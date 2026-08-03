#include <stdint.h>
#include "idt.h"

__attribute__((interrupt)) void irq0_timer(struct interrupt_frame* frame);

void     pit_init(uint32_t frequency);   // örn. 100 = 10ms tick
uint64_t timer_ticks(void);              // açılıştan beri geçen tick
void     sleep(uint32_t ms);             // meşgul-bekleme, kesme bazlı


