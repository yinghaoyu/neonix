#ifdef NEONIX
#include <neonix/stdio.h>
#include <neonix/string.h>
#include <neonix/syscall.h>
#include <neonix/types.h>
#else
#include <stdio.h>
#include <string.h>
#endif

int main(int argc, char const *argv[], char const *envp[])
{
  for (size_t i = 0; i < argc; i++)
  {
    printf("%s\n", argv[i]);
  }

  for (size_t i = 0; 1; i++)
  {
    if (!envp[i])
      break;
    int len = strlen(envp[i]);
    if (len >= 1022)
      continue;
    printf("%s\n", envp[i]);
  }
  return 0;
}