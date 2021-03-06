#ifndef NEONIX_SYSCALL_H
#define NEONIX_SYSCALL_H

#include <neonix/types.h>

typedef enum syscall_t
{
  SYS_NR_TEST,
  SYS_NR_WRITE,
  SYS_NR_SLEEP,
  SYS_NR_YIELD,
} syscall_t;

u32 test();
void yield();
void sleep(u32 ms);

int32 write(fd_t fd, char *buf, u32 len);

#endif
