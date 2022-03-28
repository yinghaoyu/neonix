[bits 32]

extern console_init

extern gdt_init
extern memory_init
extern kernel_init

global _start:
_start:
  ; mov byte [0xb8000], 'K'
  ;xchg bx, bx
  push ebx; loader中的ards_count
  push eax; loader中的magic

  call console_init; 控制台初始化
  call gdt_init; 全局描述符初始化
  call memory_init; 内存初始化
  call kernel_init; 内核初始化
  ;int 0x80  ; 系统调用0x80 中断函数 系统调用
  ;xchg bx, bx
  ; 制造除0异常
  ;mov bx, 0 
  ;div bx
  jmp $; 阻塞
