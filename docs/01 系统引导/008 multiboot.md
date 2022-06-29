## multiboot2 头

要支持 `multiboot2`，内核需添加一个 `multiboot` 头，而且必须在内核开始的 `32768(0x8000)` 字节，同时必须以 `64` 字节对齐；

| 偏移   | 类型   | 名称                     | 备注
| ------ | ------ | ------                   | --   |
| 0      | u32    | 魔数 (magic)             | 必须
| 4      | u32    | 架构 (architecture)      | 必须 |
| 8      | u32    | 头部长度 (header_length) | 必须 |
| 12     | u32    | 校验和 (checksum)        | 必须 |
| 16-XX  |        | 标记 (tags)              | 必须 |

- `magic` = 0xE85250D6
- `architecture`:
  - 0：32 位保护模式
- `checksum`：与 `magic`, `architecture`, `header_length` 相加必须为 `0`

## multiboot2 引导

### i386 状态

- EAX：魔数 `0x36d76289`
- EBX：包含 bootloader 存储 multiboot2 信息结构体的，32 位 物理地址
- CS：32 位 可读可执行的代码段，尺寸 4G
- DS/ES/FS/GS/SS：32 位可读写的数据段，尺寸 4G
- A20 线：启用
- CR0：PG = 0, PE = 1，其他未定义
- EFLAGS：VM = 0, IF = 0, 其他未定义
- ESP：内核必须尽早切换栈顶地址
- GDTR：内核必须尽早使用自己的全局描述符表
- IDTR：内核必须在设置好自己的中断描述符表之前关闭中断

Virtual 8086 Mode

## 参考文献

- [grub手册](https://www.gnu.org/software/grub/manual/grub/grub.html)
- [multiboot2规范](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.pdf)
- [multiboot2头](https://intermezzos.github.io/book/first-edition/multiboot-headers.html)
- [multiboot例子](https://os.phil-opp.com/multiboot-kernel/)
- [bochs选项](https://bochs.sourceforge.io/doc/docbook/user/bios-tips.html)
- [bochs用cdrom启动boot](https://forum.osdev.org/viewtopic.php?f=1&t=18171)
- [qemu选项](https://wiki.gentoo.org/wiki/QEMU/Options)
- [grub架构](https://hugh712.gitbooks.io/grub/content/)
