#include <neonix/assert.h>
#include <neonix/debug.h>
#include <neonix/interrupt.h>
#include <neonix/syscall.h>
#include <neonix/task.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define SYSCALL_SIZE 64

handler_t syscall_table[SYSCALL_SIZE];

void syscall_check(u32 nr)
{
  if (nr >= SYSCALL_SIZE)
  {
    panic("syscall nr error!!!");
  }
}

static void sys_default()
{
  panic("syscall not implemented!!!");
}

task_t *task = NULL;

static u32 sys_test()
{
  // LOGK("syscall test...\n");
  char *ptr;
  BMB;
  // 这里虚拟地址没有链接物理页，直接写会触发缺页异常，由CPU的MMU的硬件触发
  // ptr = (char *) 0x1600000;
  // ptr[3] = 'T';

  // BMB;

  link_page(0x1600000);

  BMB;

  ptr = (char *) 0x1600000;
  ptr[3] = 'T';
  BMB;

  unlink_page(0x1600000);

  BMB;
  return 255;
}

int32 sys_write(fd_t fd, char *buf, u32 len)
{
  if (fd == stdout || fd == stderr)
  {
    return console_write(buf, len);
  }
  // todo
  panic("write!!!!");
  return 0;
}

void syscall_init()
{
  for (size_t i = 0; i < SYSCALL_SIZE; i++)
  {
    syscall_table[i] = sys_default;
  }

  syscall_table[SYS_NR_TEST] = sys_test;
  syscall_table[SYS_NR_SLEEP] = task_sleep;
  syscall_table[SYS_NR_YIELD] = task_yield;
  syscall_table[SYS_NR_WRITE] = sys_write;
}
