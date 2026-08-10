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
void    schedule(void);
void    task_yield(void);
void    task_exit(void);

extern task_t* current_task;