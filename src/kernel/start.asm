[bits 32]

extern console_init
extern gdt_init
extern memory_init
extern kernel_init

global _start:
_start:
  push ebx; loader中的ards_count
  push eax; loader中的magic

  call console_init; 控制台初始化
  call gdt_init; 全局描述符初始化
  call memory_init; 内存初始化
  call kernel_init; 内核初始化
  jmp $; 阻塞
