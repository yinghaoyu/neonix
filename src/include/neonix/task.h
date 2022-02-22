#ifndef NEONIX_TASK_H
#define NEONIX_TASK_H

#include <neonix/types.h>

typedef u32 target_t();

typedef struct task_t
{
    u32 *stack; // 内核栈
} task_t;

typedef struct task_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);
} task_frame_t;

void task_init();

#endif
