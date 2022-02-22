#include <neonix/neonix.h>
#include <neonix/types.h>
#include <neonix/io.h>
#include <neonix/string.h>
#include <neonix/console.h>
#include <neonix/printk.h>

char message[] = "hello world!\n";

void kernel_init()
{
  console_init();
 // u32 count = 20;
 // while(count--)
 // {
 //   console_write(message, sizeof(message)-1);
 // }
  int cnt = 30;
  while(cnt--)
  {
    printk("hello neonix %#010x\n", cnt);
  }
  return;
}
