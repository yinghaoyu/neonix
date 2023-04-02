#include <neonix/stdio.h>
#include <neonix/syscall.h>
#include <neonix/types.h>

int main()
{
  if (getpid() != 1)
  {
    printf("init already running...\n");
    return 0;
  }

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
  return 0;
}