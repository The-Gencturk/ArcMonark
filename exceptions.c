#include "idt.h"
#include "panic.h"

#define EXC_NOERR(name, msg) \
    __attribute__((interrupt)) \
    static void name(struct interrupt_frame* frame) { panic(msg, frame, 0); }

#define EXC_ERR(name, msg) \
    __attribute__((interrupt)) \
    static void name(struct interrupt_frame* frame, uint64_t err) { panic(msg, frame, err); }

EXC_NOERR(exc0, "#DE Divide Error")
EXC_NOERR(exc1, "#DB Debug")
EXC_NOERR(exc2, "NMI")
EXC_NOERR(exc3, "#BP Breakpoint")
EXC_NOERR(exc4, "#OF Overflow")
EXC_NOERR(exc5, "#BR BOUND Range Exceeded")
EXC_NOERR(exc6, "#UD Invalid Opcode")
EXC_NOERR(exc7, "#NM Device Not Available")
EXC_ERR(exc8, "#DF Double Fault")
EXC_NOERR(exc9, "Coprocessor Segment Overrun")
EXC_ERR(exc10, "#TS Invalid TSS")
EXC_ERR(exc11, "#NP Segment Not Present")
EXC_ERR(exc12, "#SS Stack-Segment Fault")
EXC_ERR(exc13, "#GP General Protection Fault")
EXC_ERR(exc14, "#PF Page Fault")
EXC_NOERR(exc15, "Reserved (15)")
EXC_NOERR(exc16, "#MF x87 FPU Error")
EXC_ERR(exc17, "#AC Alignment Check")
EXC_NOERR(exc18, "#MC Machine Check")
EXC_NOERR(exc19, "#XM SIMD FP Exception")
EXC_NOERR(exc20, "#VE Virtualization")
EXC_ERR(exc21, "#CP Control Protection")
EXC_NOERR(exc22, "Reserved (22)")
EXC_NOERR(exc23, "Reserved (23)")
EXC_NOERR(exc24, "Reserved (24)")
EXC_NOERR(exc25, "Reserved (25)")
EXC_NOERR(exc26, "Reserved (26)")
EXC_NOERR(exc27, "Reserved (27)")
EXC_NOERR(exc28, "#HV Hypervisor Injection")
EXC_ERR(exc29, "#VC VMM Communication")
EXC_ERR(exc30, "#SD Security Exception")
EXC_NOERR(exc31, "Reserved (31)")

void exceptions_install(void) {
    void* h[32] = {
        exc0,  exc1,  exc2,  exc3,  exc4,  exc5,  exc6,  exc7,
        exc8,  exc9,  exc10, exc11, exc12, exc13, exc14, exc15,
        exc16, exc17, exc18, exc19, exc20, exc21, exc22, exc23,
        exc24, exc25, exc26, exc27, exc28, exc29, exc30, exc31,
    };
    for (int i = 0; i < 32; i++)
        idt_set_gate(i, h[i], IDT_INT_GATE);
}