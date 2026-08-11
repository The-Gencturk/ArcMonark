#include "timer.h"
#include "idt.h"
#include "pic.h"
#include "io.h"
#include "sched.h"

#define PIT_CHANNEL0   0x40
#define PIT_COMMAND    0x43
#define PIT_FREQUENCY  1193182   

static volatile uint64_t ticks = 0;
static uint32_t          tick_hz = 100;

__attribute__((interrupt))
void irq0_timer(struct interrupt_frame* frame) {
    (void)frame;
    ticks++;
    pic_send_eoi(0);
    sched_tick();          // preemptive zaman-dilimi anahtari (tek task ise etkisiz)
}

void pit_init(uint32_t frequency) {
    tick_hz = frequency;
    uint32_t divisor = PIT_FREQUENCY / frequency;


    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));         
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));  
}

uint64_t timer_ticks(void) {
    return ticks;
}

void sleep(uint32_t ms) {
    uint64_t target = ticks + (ms * tick_hz) / 1000;
    while (ticks < target)
        asm volatile("hlt");   
}