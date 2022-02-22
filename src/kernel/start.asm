[bits 32]

extern kernel_init

global _start:
_start:
  ; mov byte [0xb8000], 'K'
  ;xchg bx, bx
  call kernel_init
  int 0x80  ; 系统调用0x80 中断函数 系统调用
  ;xchg bx, bx
  jmp $
