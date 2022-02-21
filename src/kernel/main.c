#include "../include/neoinx/neoinx.h"
#include "../include/neoinx/types.h"
#include "../include/neoinx/io.h"
#include "../include/neoinx/string.h"
#include "../include/neoinx/console.h"

char message[] = "hello world!\n";
char o[] = "neoinx\n";

void kernel_init()
{
  console_init();
  u32 count = 20;
  while(count--)
  {
    console_write(message, sizeof(message)-1);
  }
  return;
}
