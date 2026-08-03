#pragma once
#include <stdint.h>

#define PIC1_COMMAND  0x20
#define PIC1_DATA     0x21
#define PIC2_COMMAND  0xA0
#define PIC2_DATA     0xA1


#define PIC1_OFFSET   0x20   // IRQ 0-7  -> vektör 0x20-0x27
#define PIC2_OFFSET   0x28   // IRQ 8-15 -> vektör 0x28-0x2F

void pic_remap(void);
void pic_send_eoi(uint8_t irq);
void irq_set_mask(uint8_t irq);    
void irq_clear_mask(uint8_t irq);   