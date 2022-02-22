#include <neonix/debug.h>
#include <neonix/stdarg.h>
#include <neonix/stdio.h>
#include <neonix/printk.h>

static char buf[1024];

void debugk(char* file, int line, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  printk("[%s] [%d] %s", file, line, buf);
}
