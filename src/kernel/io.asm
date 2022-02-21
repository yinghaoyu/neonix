[bits 32]

section .text ; 代码段

global inb  ; 将inb导出，链接时能找到这个函数
inb:
  push ebp
  mov ebp, esp  ; 保存栈帧

  xor eax, eax
  mov edx, [ebp + 8]  ; port
  in al, dx
  
  jmp $+2
  jmp $+2
  jmp $+2


  leave  ; 恢复栈帧

  ret

global outb
outb:
  push ebp
  mov ebp, esp  ; 保存栈帧

  mov edx, [ebp + 8]  ; port
  mov eax, [ebp + 12]  ; value
  out dx, al  ; 将value的8bit输出到port

  jmp $+2
  jmp $+2
  jmp $+2

  leave  ; 恢复栈帧

  ret

global inw
inw:
  push ebp
  mov ebp, esp  ; 保存栈帧

  xor eax, eax
  mov edx, [ebp + 8]  ; port
  in ax, dx

  jmp $+2
  jmp $+2
  jmp $+2


  leave  ; 恢复栈帧

  ret

global outw
outw:
  push ebp
  mov ebp, esp  ; 保存栈帧

  mov edx, [ebp + 8]  ; port
  mov eax, [ebp + 12]  ; value
  out dx, ax  ; 将value的16bit输出到port

  jmp $+2
  jmp $+2
  jmp $+2

  leave  ; 恢复栈帧

  ret
