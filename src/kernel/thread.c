#include <neonix/arena.h>
#include <neonix/debug.h>
#include <neonix/fs.h>
#include <neonix/interrupt.h>
#include <neonix/stdio.h>
#include <neonix/string.h>
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

static void user_init_thread()
{
  while (true)
  {
    u32 status;
    pid_t pid = fork();
    if (pid)
    {
      pid_t child = waitpid(pid, &status);
      printf("wait pid %d status %d %d\n", child, status, time());
    }
    else
    {
      int err = execve("/bin/neosh.out", NULL, NULL);
      printf("execve /bin/neosh.out error %d\n", err);
      exit(err);
    }
  }
}

extern void dev_init();

void init_thread()
{
  char temp[100];  // 保证栈顶有足够的空间
  dev_init();
  task_to_user_mode(user_init_thread);
}

void test_thread()
{
  set_interrupt_state(true);
  while (true)
  {
    sleep(10);
  }
}
