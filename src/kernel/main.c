#include "../include/neoinx/neoinx.h"

int magic = ONIX_MAGIC;
char message[] = "hello neoinx!";  // .data
char buff[1024];  // .bss

void kernel_init()
{
  char* video = (char*) 0xb8000;  // 文本显示器地址
  for(int i = 0; i < sizeof(message); ++i)
  {
    video[i * 2] = message[i];
  }
}
