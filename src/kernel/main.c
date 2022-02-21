#include "../include/neonix/neonix.h"
#include "../include/neonix/types.h"
#include "../include/neonix/io.h"
#include "../include/neonix/string.h"
#include "../include/neonix/console.h"
#include "../include/neonix/printk.h"

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
