#include <neonix/debug.h>
#include <neonix/device.h>
#include <neonix/printk.h>
#include <neonix/stdarg.h>
#include <neonix/stdio.h>

static char buf[1024];

void debugk(char *file, int line, const char *fmt, ...)
{
  device_t *device = device_find(DEV_SERIAL, 0);
  if (!device)
  {
    device = device_find(DEV_CONSOLE, 0);
  }

  int i = sprintf(buf, "[%s] [%d] ", file, line);
  device_write(device->dev, buf, i, 0, 0);

  va_list args;
  va_start(args, fmt);
  i = vsprintf(buf, fmt, args);
  va_end(args);

  device_write(device->dev, buf, i, 0, 0);
}
