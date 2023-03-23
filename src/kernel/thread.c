#include <neonix/arena.h>
#include <neonix/debug.h>
#include <neonix/interrupt.h>
#include <neonix/stdio.h>
#include <neonix/syscall.h>
#include <neonix/task.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

void idle_thread()
{
  set_interrupt_state(true);
  u32 counter = 0;
  while (true)
  {
    // LOGK("idle task.... %d\n", counter++);
    asm volatile(
        "sti\n"  // 开中断
        "hlt\n"  // 关闭 CPU，进入暂停状态，等待外中断的到来
    );
    yield();  // 放弃执行权，调度执行其他任务
  }
}

#include <neonix/mutex.h>

mutex_t mutex;
lock_t lock;

void user_init_thread()
{
  // mutex_init(&mutex);
  // lock_init(&lock);
  // set_interrupt_state(true);
  u32 counter = 0;

  char ch;

  while (true)
  {
    sleep(1000);
    printf("task is in user mode %d\n", counter++);
  }
}

void init_thread()
{
  char temp[100];  // 保证栈顶有足够的空间
  task_to_user_mode(user_init_thread);
}

void test_thread()
{
  set_interrupt_state(true);
  u32 counter = 0;

  while (true)
  {
    // LOGK("test task %d....\n", counter++);
    void *ptr = kmalloc(1200);
    LOGK("kmalloc 0x%p....\n", ptr);
    kfree(ptr);

    ptr = kmalloc(1024);
    LOGK("kmalloc 0x%p....\n", ptr);
    kfree(ptr);

    ptr = kmalloc(54);
    LOGK("kmalloc 0x%p....\n", ptr);
    kfree(ptr);

    sleep(5000);
  }
}
