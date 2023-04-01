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
  char buf[256];
  memset(buf, 'A', sizeof(buf));

  fd_t fd;
  int len = 0;
  fd = open("/hello.txt", O_RDWR, 0755);
  lseek(fd, 5, SEEK_SET);
  len = write(fd, buf, sizeof(buf));
  close(fd);
  while (true)
  {
    char ch;
    read(stdin, &ch, 1);
    write(stdout, &ch, 1);
    sleep(10);
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
  while (true)
  {
    sleep(10);
  }
}
