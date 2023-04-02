#include <neonix/signal.h>
#include <neonix/stdlib.h>
#include <neonix/syscall.h>

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    return -1;
  }
  return kill(atoi(argv[1]), SIGTERM);
}