#include <neonix/fs.h>
#include <neonix/stdio.h>
#include <neonix/syscall.h>

int main(int argc, char const *argv[])
{
  char ch;
  while (true)
  {
    int n = read(STDIN_FILENO, &ch, 1);
    if (n == EOF)
    {
      break;
    }
    if (ch == '\n')
    {
      write(STDOUT_FILENO, &ch, 1);
      break;
    }
    write(STDOUT_FILENO, &ch, 1);
    write(STDOUT_FILENO, &ch, 1);
  }
  return 0;
}