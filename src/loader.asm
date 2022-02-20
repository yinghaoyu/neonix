[org 0x1000]

dw 0x55aa; 魔数，用于判断错误

; 打印字符串
mov si, loading
call print

xchg bx, bx ; breakpoint

detect_memory:
  ; 将ebx清0
  xor ebx, ebx

  ; es:di结构体缓存位置
  mov ax, 0
  mov es, ax
  mov edi, ards_buffer
  
  mov edx, 0x534d4150  ; 固定签名

.next
  ; 子功能号
  mov eax, 0xe820
  ; ards 结构大小，20字节
  mov ecx, 20
  ; 调用0x15系统调用
  int 0x15
  
  ; 如果CF置位，表示出错
  jc error

  ; 将缓存指针指向下一个结构体
  add di, cx
  
  ; 将结构体数量加1
  inc word [ards_count]

  cmp ebx, 0
  jnz .next

  mov si, deceting
  call print  ; 从实模式进入保护模式print函数就不能用了

  xchg bx, bx  ; breakpoint
  ; mov byte [0xb8000], 'P' ;  实模式不能修改1M以外的内存
  jmp prepare_protect_mode

prepare_protect_mode:
  xchg bx, bx

  cli  ; 关中断

  ; 打开A20线
  in al, 0x92
  or al, 0b10
  out 0x92, al

  ; 加载gdt
  lgdt [gdt_ptr]

  ; 启动保护模式cr0的0位置1
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  ; 用跳转来刷新缓存,启用保护模式
  jmp dword code_selector:protect_mode

print:
  mov ah, 0x0e
.next:
  mov al, [si]
  cmp al, 0
  jz .done
  int 0x10
  inc si
  jmp .next
.done:
  ret

; 数据区最好放在代码区后面
loading:
  db "Loading Neonix...", 10, 13, 0; \n\r

deceting:
  db "Deceting Memory Success...", 10, 13, 0; \n\r

error:
  mov si, .msg
  call print
  hlt; 让 CPU 停止
  jmp $
  .msg db "Loading Error!!!", 10, 13, 0

[bits 32]  ; 从以下开始，提示编译器地址变32位
protect_mode:
  xchg bx, bx
  mov ax, data_selector
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax ; 初始化段寄存器

  mov esp, 0x10000  ; 修改栈顶

  ; mov byte [0xb8000], 'P' ; 把屏幕字符B改为P

  ; mov byte [0x200000], 'P' ; 地址2M处

jmp $

; 段选择子
code_selector equ (1 << 3)
data_selector equ (2 << 3)

memory_base equ 0  ; 内存开始的位置:基地址

; 内存界限 4G / 4K -1
memory_limit equ (1024 * 1024 * 1024 *4) / (1024 * 4) - 1

gdt_ptr:
  dw (gdt_end - gdt_base) - 1
  dd gdt_base
gdt_base:
  dd 0, 0  ; NULL描述符
gdt_code:
  dw memory_limit & 0xffff  ; 段界限的0～15位
  dw memory_base & 0xffff  ; 基地址0～15位
  db (memory_base >> 16) & 0xff  ; 基地址16～23位
  ; present 1(在内存中) , dpl 00(特权级别), s 1(代码段或数据段), type(代码段, 不可依从,可读,未被访问)
  db 0b_1_00_1_1_0_1_0
  ; 段界限16～19位, 4k-32-不是64位-
  db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
  db (memory_base >> 24) & 0xff  ; 基地址 24～31位
gdt_data:
  dw memory_limit & 0xffff  ; 段界限的0～15位
  dw memory_base & 0xffff  ; 基地址0～15位
  db (memory_base >> 16) & 0xff  ; 基地址16～23位
  ; present 1(在内存中) , dpl 00(特权级别), s 1(代码段或数据段), type(数据段, 向上扩展,可写,未被访问)
  db 0b_1_00_1_0_0_1_0
  ; 段界限16～19位, 4k-32-不是64位-
  db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
  db (memory_base >> 24) & 0xff  ; 基地址 24～31位
gdt_end:

; buffer放后面是为了避免溢出
ards_count:
  dw 0
ards_buffer:
  
