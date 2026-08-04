#pragma once
#include <stdint.h>

#define IDT_ENTRIES   256
#define KERNEL_CS  0x28

struct idt_entry {
    uint16_t offset_low;    
    uint16_t selector;      
    uint8_t  ist;           
    uint8_t  type_attr;     
    uint16_t offset_mid;   
    uint32_t offset_high;   
    uint32_t zero;          
} __attribute__((packed));

struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

struct idtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));


#define IDT_INT_GATE   0x8E  
#define IDT_TRAP_GATE  0x8F   
#define IDT_INT_USER   0xEE   

void idt_set_gate(uint8_t vec, void* handler, uint8_t flags);
void idt_init(void);