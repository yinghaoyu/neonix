#ifndef NEONIX_SYSCALL_H
#define NEONIX_SYSCALL_H

#include <neonix/types.h>

typedef enum syscall_t
{
  SYS_NR_TEST,
  SYS_NR_WRITE = 4,
  SYS_NR_BRK = 45,
  SYS_NR_YIELD = 158,
  SYS_NR_SLEEP = 162,
} syscall_t;

u32 test();
void yield();
void sleep(u32 ms);

int32 brk(void *addr);

int32 write(fd_t fd, char *buf, u32 len);

#endif
