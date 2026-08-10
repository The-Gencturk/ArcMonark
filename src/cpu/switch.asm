bits 64
section .text

global switch_context
global task_trampoline
extern task_exit

; void switch_context(uint64_t* out_old_rsp, uint64_t new_rsp)
;   rdi = &prev->rsp   
;   rsi =  next->rsp   
switch_context:
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov [rdi], rsp      ; prev->rsp = rsp
    mov rsp, rsi        ; rsp = next->rsp

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret                 ; mevcut task: schedule()'a döner
                        ; taze task: task_trampoline'a atlar

; Yeni task'ın ilk kez çalıştığı yer. Fake frame'de rbx/r12 öyle kuruldu.
task_trampoline:
    sti                 ; taze task interrupt'ları AÇIK başlamalı (kritik!)
    mov rdi, r12        ; arg -> 1. parametre
    call rbx            ; entry(arg)
    call task_exit      ; entry dönerse task'ı öldür
.hang:
    cli
    hlt
    jmp .hang