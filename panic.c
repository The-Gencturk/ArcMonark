#include "panic.h"
#include "fb.h"

static void print_hex(uint64_t val) {
    char buf[19];
    const char* h = "0123456789ABCDEF";
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; i++)
        buf[2 + i] = h[(val >> ((15 - i) * 4)) & 0xF];
    buf[18] = '\0';
    print(buf);          // <-- print, putchar DEGIL
}
__attribute__((noreturn))
void panic(const char* msg, struct interrupt_frame* frame, uint64_t error_code) {
    asm volatile("cli");
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));

    set_color(0xFFFFFFFF, 0x00AA0000);
    clear_screen();

    print("*** KERNEL PANIC ***\n\n");
    print(msg); print("\n\n");

    print("RIP    = "); print_hex(frame->rip);    print("\n");
    print("CS     = "); print_hex(frame->cs);     print("\n");
    print("RFLAGS = "); print_hex(frame->rflags); print("\n");
    print("RSP    = "); print_hex(frame->rsp);    print("\n");
    print("SS     = "); print_hex(frame->ss);     print("\n");
    print("ERRCODE= "); print_hex(error_code);    print("\n");
    print("CR2    = "); print_hex(cr2);           print("  (sadece #PF icin)\n\n");

    print("Sistem donduruldu.\n");
    for (;;) asm volatile("hlt");
}