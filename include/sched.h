#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

typedef enum { TASK_READY, TASK_RUNNING, TASK_DEAD } task_state_t;

typedef struct task {
    uint64_t     rsp;
    uint64_t* stack_base;
    task_state_t state;
    uint64_t     id;
    struct task* next;
} task_t;

void    sched_init(void);
task_t* task_create(void (*entry)(void*), void* arg);
void    schedule(void);      // siradaki hazir task'a gecir (switch)
void    task_yield(void);    // gonullu birak
void    task_exit(void);     // mevcut task'i sonlandir (stack sonra geri alinir)

// Timer IRQ'sundan cagrilir: preemptive zaman-dilimi anahtari.
void    sched_tick(void);

// Kac task hayatta (RUNNING/READY). Demo/teshis icin.
uint32_t sched_task_count(void);

extern task_t* current_task;

#endif
