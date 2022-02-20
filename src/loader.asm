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
  call print

  ;xchg bx, bx ; breakpoint

  ; 结构体数量
  ;mov cx, [ards_count]
  ; 结构体指针
  ;mov si, 0
;.show:
;  mov eax, [ards_buffer + esi]
;  mov ebx, [ards_buffer + esi + 8]
;  mov edx, [ards_buffer + esi + 16]
;  add si, 20
;  xchg bx, bx
;  loop .show

jmp $

; xchg bx, bx; 断点

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

; buffer放后面是为了避免溢出
ards_count:
  dw 0
ards_buffer:
  
