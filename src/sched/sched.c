#include "sched.h"
#include "heap.h"   

#define KSTACK_SIZE 0x4000  

extern void switch_context(uint64_t* out_old_rsp, uint64_t new_rsp);
extern void task_trampoline(void);

task_t* current_task = NULL;
static task_t* head = NULL;       
static uint64_t next_tid = 0;

static void list_add(task_t* t) {
    t->next = head->next;       
    head->next = t;
}

void sched_init(void) {
    task_t* boot = kmalloc(sizeof(task_t));
    boot->id = next_tid++;
    boot->state = TASK_RUNNING;
    boot->stack_base = NULL;      
    boot->next = boot;            
    head = boot;
    current_task = boot;
}

task_t* task_create(void (*entry)(void*), void* arg) {
    task_t* t = kmalloc(sizeof(task_t));
    uint8_t* stack = kmalloc(KSTACK_SIZE);

    uint64_t  top = ((uint64_t)(stack + KSTACK_SIZE)) & ~0xFULL; 
    uint64_t* sp = (uint64_t*)top;

    *(--sp) = (uint64_t)task_trampoline; 
    *(--sp) = 0;                       
    *(--sp) = (uint64_t)entry;           
    *(--sp) = (uint64_t)arg;         
    *(--sp) = 0;                   
    *(--sp) = 0;                         
    *(--sp) = 0;                     

    t->rsp = (uint64_t)sp;
    t->stack_base = (uint64_t*)stack;
    t->state = TASK_READY;
    t->id = next_tid++;

    list_add(t);
    return t;
}

void schedule(void) {
    if (!current_task) return;

    task_t* prev = current_task;
    task_t* next = prev->next;
    while (next->state == TASK_DEAD && next != prev)
        next = next->next;

    if (next == prev) return;     

    if (prev->state == TASK_RUNNING) prev->state = TASK_READY;
    next->state = TASK_RUNNING;
    current_task = next;

    switch_context(&prev->rsp, next->rsp);

}

void task_yield(void) {       
    __asm__ volatile("cli");
    schedule();
    __asm__ volatile("sti");
}

void task_exit(void) {
    __asm__ volatile("cli");
    current_task->state = TASK_DEAD;
    schedule();               
    for (;;) __asm__ volatile("hlt");
}