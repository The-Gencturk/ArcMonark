#include "sched.h"
#include "heap.h"
#include "irqlock.h"

#define KSTACK_SIZE 0x4000

extern void switch_context(uint64_t* out_old_rsp, uint64_t new_rsp);
extern void task_trampoline(void);

task_t* current_task = NULL;
static task_t* head = NULL;         // dairesel listenin sabit dugumu (boot task)
static uint64_t next_tid = 0;
static task_t* zombie = NULL;       // sonlanmis, stack'i geri alinmayi bekleyen task

static void list_add(task_t* t) {
    t->next = head->next;
    head->next = t;
}

// t'yi dairesel listeden cikar (onceki dugumu bulup baglantiyi atlar).
static void list_remove(task_t* t) {
    task_t* p = t->next;
    while (p->next != t) p = p->next;
    p->next = t->next;
}

// Sonlanmis task'in stack + struct'ini geri al. Uzerinde calismadigimizdan
// emin olmak icin zombie != current_task kontrolu yapilir. Heap islemleri
// kesmeye karsi kilitli oldugundan buradan (timer yolu dahil) cagrilmasi guvenli.
static void reap(void) {
    if (zombie && zombie != current_task) {
        task_t* z = zombie;
        zombie = NULL;
        if (z->stack_base) kfree(z->stack_base);
        kfree(z);
    }
}

void sched_init(void) {
    task_t* boot = kmalloc(sizeof(task_t));
    boot->id = next_tid++;
    boot->state = TASK_RUNNING;
    boot->stack_base = NULL;         // boot stack heap'ten degil; geri alinmaz
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

    uint64_t f = irq_save();
    list_add(t);
    irq_restore(f);
    return t;
}

void schedule(void) {
    if (!current_task) return;

    reap();                          // bekleyen zombie varsa geri al

    task_t* prev = current_task;
    task_t* next = prev->next;
    while (next->state == TASK_DEAD && next != prev)
        next = next->next;

    if (next == prev) return;        // tek hazir task: anahtarlama yok

    if (prev->state == TASK_RUNNING) prev->state = TASK_READY;
    next->state = TASK_RUNNING;
    current_task = next;

    switch_context(&prev->rsp, next->rsp);
}

// Timer IRQ'sundan gelir. Kesme zaten kapali oldugundan ekstra kilit gerekmez;
// schedule() switch_context yapar, task kendi stack'inde iretq'e geri doner.
void sched_tick(void) {
    schedule();
}

void task_yield(void) {
    uint64_t f = irq_save();
    schedule();
    irq_restore(f);
}

void task_exit(void) {
    __asm__ volatile("cli");
    current_task->state = TASK_DEAD;
    list_remove(current_task);       // listeden cikar
    zombie = current_task;           // stack'i baska task geri alsin
    schedule();                      // baska task'a gecir; buraya donmez
    for (;;) __asm__ volatile("hlt");
}

uint32_t sched_task_count(void) {
    uint64_t f = irq_save();
    uint32_t n = 0;
    task_t* t = head;
    do {
        if (t->state != TASK_DEAD) n++;
        t = t->next;
    } while (t != head);
    irq_restore(f);
    return n;
}
