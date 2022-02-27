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
#include <neonix/stdlib.h>

char message[] = "hello world!\n";

void kernel_init()
{
  console_init();
  //int cnt = 30;
  //while(cnt--)
  //{
  //  printk("hello neonix %#010x\n", cnt);
  //}
  gdt_init();
  interrupt_init();
  //task_init();
  //assert(3 > 5);
  asm volatile(
      "sti\n"
      "mov %eax, %eax\n");

  u32 counter = 0;
  while(true)
  {
    DEBUGK("looping in kernel_init %d...\n", counter++);
    delay(100000000);
  }

  return;
}
