#include "../include/neoinx/console.h"
#include "../include/neoinx/io.h"

#define CRT_ADDR_REG 0x3D4 // CRT(6845)索引寄存器
#define CRT_DATA_REG 0x3D5 // CRT(6845)数据寄存器

#define CRT_START_ADDR_H 0xC // 显示内存起始位置 - 高位
#define CRT_START_ADDR_L 0xD // 显示内存起始位置 - 低位
#define CRT_CURSOR_H 0xE     // 光标位置 - 高位
#define CRT_CURSOR_L 0xF     // 光标位置 - 低位

#define MEM_BASE 0xB8000              // 显卡内存起始位置
#define MEM_SIZE 0x4000               // 显卡内存大小,16KB
#define MEM_END (MEM_BASE + MEM_SIZE) // 显卡内存结束位置
#define WIDTH 80                      // 屏幕文本列数
#define HEIGHT 25                     // 屏幕文本行数
#define ROW_SIZE (WIDTH * 2)          // 每行字节数,前一个字节是样式,后一个字节是ASCii码
#define SCR_SIZE (ROW_SIZE * HEIGHT)  // 屏幕字节数

#define ASCII_NUL 0x00
#define ASCII_ENQ 0x05
#define ASCII_BEL 0x07 // \a
#define ASCII_BS 0x08  // \b
#define ASCII_HT 0x09  // \t
#define ASCII_LF 0x0A  // \n
#define ASCII_VT 0x0B  // \v
#define ASCII_FF 0x0C  // \f
#define ASCII_CR 0x0D  // \r
#define ASCII_DEL 0x7F

static u32 screen;  // 当前显示器开始的内存位置

static u32 pos;  // 记录当前光标的内存位置

static u32 x, y;  // 记录当前光标的坐标

static u8 attr = 7;  // 字符样式
static u16 erase = 0x0720;  // 空格ASCii 20, 07是字符样式

// 获取当前显示器开始的位置
static void get_screen()
{
  outb(CRT_ADDR_REG, CRT_START_ADDR_H);  // 开始位置高8位
  screen = inb(CRT_DATA_REG) << 8;
  outb(CRT_ADDR_REG, CRT_START_ADDR_L);  // 开始位置低8位
  screen |= inb(CRT_DATA_REG);

  screen <<= 1;  // screen*2 两个字节是一个字符
  screen += MEM_BASE;  // 得到显示器内存开始的位置
}

// 设置当前显示器开始的位置
static void set_screen()
{
  outb(CRT_ADDR_REG, CRT_START_ADDR_H);  // 开始位置高8位
  outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 9) & 0xff);  // 先>>1,再>>8,&0xff是为了确保高位都是0
  outb(CRT_ADDR_REG, CRT_START_ADDR_L);  // 开始位置低8位
  outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 1) & 0xff);
}

// 获取当前光标的位置
static void get_cursor()
{
  outb(CRT_ADDR_REG, CRT_CURSOR_H);  // 开始位置高8位
  pos = inb(CRT_DATA_REG) << 8;
  outb(CRT_ADDR_REG, CRT_CURSOR_L);  // 开始位置低8位
  pos |= inb(CRT_DATA_REG);

  get_screen();

  pos <<= 1;  // pos * 2

  pos += MEM_BASE;  // 得到光标内存开始的位置

  u32 delta = (pos - screen) >> 1;

  x = delta % WIDTH;
  y = delta / WIDTH;
}

static void set_cursor()
{
  outb(CRT_ADDR_REG, CRT_CURSOR_H);  // 开始位置高8位
  outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 9) & 0xff);
  outb(CRT_ADDR_REG, CRT_CURSOR_L);  // 开始位置低8位
  outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 1) & 0xff);
}

void console_clear()
{
  screen = MEM_BASE;
  pos = MEM_BASE;
  x = y = 0;
  set_cursor();
  set_screen();

  u16* ptr = (u16 *)MEM_BASE;
  while(ptr < MEM_END)
  {
    *ptr++ = erase;  // 显存全部填空格
  }
}

// 向上滚屏
static void scroll_up()
{
  if (screen + SCR_SIZE + ROW_SIZE < MEM_END)  // 屏幕结束地址+新行大小 未超过显存结束地址
  {
    u32 *ptr = (u32 *)(screen + SCR_SIZE);
    for (size_t i = 0; i < WIDTH; i++)
    {
      *ptr++ = erase;  // 新行补上空格
    }
    screen += ROW_SIZE;
    pos += ROW_SIZE;
  }
  else
  {
    memcpy((void *)MEM_BASE, (void *)screen, SCR_SIZE);  // 重新定位
    pos -= (screen - MEM_BASE);
    screen = MEM_BASE;
  }
  set_screen();
}

// back space
static void command_bs()
{
  if(x)
  {
    x--;
    pos -= 2;
    *(u16*)pos = erase;  // 删除光标左侧的字符
  }
}

// /b
static void command_del()
{
  *(u16*)pos = erase;  // 删除光标右侧的字符
}

// carriage return
static void command_cr()
{
  pos -= (x << 1);  // 减去 2*x B
  x = 0;
}

// line feed
static void command_lf()
{
  if(y + 1 < HEIGHT)  // 换行后没超出
  {
    y++;
    pos += ROW_SIZE;
  }
  else
  {
    scroll_up();  // 超出就滚屏
  }
}

void console_write(char *buf, u32 count)
{
    char ch;
    while (count--)
    {
        ch = *buf++;
        switch (ch)
        {
        case ASCII_NUL:
            break;
        case ASCII_BEL:
            // todo \a
            break;
        case ASCII_BS:
            command_bs();
            break;
        case ASCII_HT:
            break;
        case ASCII_LF:
            command_lf();
            command_cr();
            break;
        case ASCII_VT:
            break;
        case ASCII_FF:
            command_lf();
            break;
        case ASCII_CR:
            command_cr();
            break;
        case ASCII_DEL:
            command_del();
            break;
        default:
            if (x >= WIDTH)
            {
                x -= WIDTH;
                pos -= ROW_SIZE;
                command_lf();
            }

            *((char *)pos) = ch;
            pos++;
            *((char *)pos) = attr;
            pos++;

            x++;
            break;
        }
    }
    set_cursor();
}

void console_init()
{
  console_clear();
}
