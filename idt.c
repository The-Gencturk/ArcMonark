#include "idt.h"
#include "pic.h"
#include "exceptions.h"

static struct idt_entry idt[IDT_ENTRIES];
static struct idtr      idtr;
    
void idt_set_gate(uint8_t vec, void* handler, uint8_t flags) {
    uint64_t addr = (uint64_t)handler;
    idt[vec].offset_low = addr & 0xFFFF;
    idt[vec].selector = KERNEL_CS;
    idt[vec].ist = 0;
    idt[vec].type_attr = flags;
    idt[vec].offset_mid = (addr >> 16) & 0xFFFF;
    idt[vec].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vec].zero = 0;
}



__attribute__((interrupt))
static void page_fault(struct interrupt_frame* frame, uint64_t error_code) {
    (void)frame; (void)error_code;
    for (;;) asm volatile("cli; hlt");
}



void idt_init(void) {
    idtr.limit = sizeof(idt) - 1;   
    idtr.base = (uint64_t)&idt;

	exceptions_install();

   
    idt_set_gate(14, (void*)page_fault, IDT_INT_GATE);

    asm volatile("lidt %0" :: "m"(idtr));
}