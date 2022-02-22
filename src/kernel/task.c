#include <neonix/task.h>
#include <neonix/printk.h>
#include <neonix/debug.h>

#define PAGE_SIZE 0x1000

task_t *a = (task_t *)0x1000;
task_t *b = (task_t *)0x2000;

extern void task_switch(task_t *next);

task_t *running_task()
{
  // movl 4字节
  // movw 2字节
  // movb 1字节
  asm volatile(
      "movl %esp, %eax\n"
      "andl $0xfffff000, %eax\n");  // %eax的内容是函数返回值,取出结构体首地址
}

void schedule()
{
  task_t *current = running_task();
  task_t *next = current == a ? b : a;
  task_switch(next);
}

u32 thread_a()
{
  while (true)
  {
    printk("A");
    schedule();
  }
}

u32 thread_b()
{
  while (true)
  {
    printk("B");
    schedule();
  }
}

static void task_create(task_t *task, target_t target)
{
  u32 stack = (u32)task + PAGE_SIZE;

  stack -= sizeof(task_frame_t);
  task_frame_t *frame = (task_frame_t *)stack;
  frame->ebx = 0x11111111;
  frame->esi = 0x22222222;
  frame->edi = 0x33333333;
  frame->ebp = 0x44444444;
  frame->eip = (void *)target;

  task->stack = (u32 *)stack;
}

void task_init()
{
  task_create(a, thread_a);
  task_create(b, thread_b);
  schedule();
}
