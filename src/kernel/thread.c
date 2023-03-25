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

void test_recursion()
{
  char tmp[0x400];
  test_recursion();
}

void user_init_thread()
{
  u32 counter = 0;

  char ch;

  while (true)
  {
    printf("task is in user mode %d\n", counter++);
    test_recursion();
    sleep(1000);
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
    LOGK("test task %d....\n", counter++);
    sleep(2000);
  }
}
