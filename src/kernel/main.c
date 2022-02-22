#include <neonix/neonix.h>
#include <neonix/types.h>
#include <neonix/io.h>
#include <neonix/string.h>
#include <neonix/task.h>
#include <neonix/console.h>
#include <neonix/printk.h>
#include <neonix/global.h>
#include <neonix/debug.h>
#include <neonix/interrupt.h>
#include <neonix/assert.h>

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
  BMB;
  gdt_init();
  interrupt_init();
  task_init();
  assert(3 > 5);
  return;
}
