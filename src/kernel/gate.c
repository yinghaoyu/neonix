#include <neonix/assert.h>
#include <neonix/buffer.h>
#include <neonix/debug.h>
#include <neonix/device.h>
#include <neonix/fs.h>
#include <neonix/interrupt.h>
#include <neonix/memory.h>
#include <neonix/string.h>
#include <neonix/syscall.h>
#include <neonix/task.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define SYSCALL_SIZE 256

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

static u32 sys_test()
{
  char ch;
  device_t *device;

  device = device_find(DEV_KEYBOARD, 0);
  assert(device);
  device_read(device->dev, &ch, 1, 0, 0);
  device = device_find(DEV_CONSOLE, 0);
  assert(device);
  device_write(device->dev, &ch, 1, 0, 0);
  return 255;
}

extern int sys_read();
extern int sys_write();
extern int32 console_write();

extern fd_t sys_open();
extern fd_t sys_creat();
extern void sys_close();
extern int sys_mkdir();
extern int sys_rmdir();
extern int sys_link();
extern int sys_unlink();
extern time_t sys_time();
extern mode_t sys_umask();

void syscall_init()
{
  for (size_t i = 0; i < SYSCALL_SIZE; i++)
  {
    syscall_table[i] = sys_default;
  }

  syscall_table[SYS_NR_TEST] = sys_test;
  syscall_table[SYS_NR_EXIT] = task_exit;
  syscall_table[SYS_NR_FORK] = task_fork;
  syscall_table[SYS_NR_WAITPID] = task_waitpid;
  syscall_table[SYS_NR_SLEEP] = task_sleep;
  syscall_table[SYS_NR_YIELD] = task_yield;
  syscall_table[SYS_NR_GETPID] = sys_getpid;
  syscall_table[SYS_NR_GETPPID] = sys_getppid;
  syscall_table[SYS_NR_BRK] = sys_brk;
  syscall_table[SYS_NR_READ] = sys_read;
  syscall_table[SYS_NR_WRITE] = sys_write;
  syscall_table[SYS_NR_MKDIR] = sys_mkdir;
  syscall_table[SYS_NR_RMDIR] = sys_rmdir;
  syscall_table[SYS_NR_OPEN] = sys_open;
  syscall_table[SYS_NR_CREAT] = sys_creat;
  syscall_table[SYS_NR_CLOSE] = sys_close;
  syscall_table[SYS_NR_LINK] = sys_link;
  syscall_table[SYS_NR_UNLINK] = sys_unlink;
  syscall_table[SYS_NR_TIME] = sys_time;

  syscall_table[SYS_NR_UMASK] = sys_umask;
}
