#include <neonix/assert.h>
#include <neonix/debug.h>
#include <neonix/fifo.h>
#include <neonix/interrupt.h>
#include <neonix/io.h>
#include <neonix/mutex.h>
#include <neonix/task.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CTRL_PORT 0x64

#define KEYBOARD_CMD_LED 0xED  // 设置 LED 状态
#define KEYBOARD_CMD_ACK 0xFA  // ACK

#define INV 0  // 不可见字符

#define CODE_PRINT_SCREEN_DOWN 0xB7
typedef enum
{
  /* 0x00 */ KEY_NONE,
  /* 0x01 */ KEY_ESC,
  /* 0x02 */ KEY_1,
  /* 0x03 */ KEY_2,
  /* 0x04 */ KEY_3,
  /* 0x05 */ KEY_4,
  /* 0x06 */ KEY_5,
  /* 0x07 */ KEY_6,
  /* 0x08 */ KEY_7,
  /* 0x09 */ KEY_8,
  /* 0x0A */ KEY_9,
  /* 0x0B */ KEY_0,
  /* 0x0C */ KEY_MINUS,
  /* 0x0D */ KEY_EQUAL,
  /* 0x0E */ KEY_BACKSPACE,
  /* 0x0F */ KEY_TAB,
  /* 0x10 */ KEY_Q,
  /* 0x11 */ KEY_W,
  /* 0x12 */ KEY_E,
  /* 0x13 */ KEY_R,
  /* 0x14 */ KEY_T,
  /* 0x15 */ KEY_Y,
  /* 0x16 */ KEY_U,
  /* 0x17 */ KEY_I,
  /* 0x18 */ KEY_O,
  /* 0x19 */ KEY_P,
  /* 0x1A */ KEY_BRACKET_L,
  /* 0x1B */ KEY_BRACKET_R,
  /* 0x1C */ KEY_ENTER,
  /* 0x1D */ KEY_CTRL_L,
  /* 0x1E */ KEY_A,
  /* 0x1F */ KEY_S,
  /* 0x20 */ KEY_D,
  /* 0x21 */ KEY_F,
  /* 0x22 */ KEY_G,
  /* 0x23 */ KEY_H,
  /* 0x24 */ KEY_J,
  /* 0x25 */ KEY_K,
  /* 0x26 */ KEY_L,
  /* 0x27 */ KEY_SEMICOLON,
  /* 0x28 */ KEY_QUOTE,
  /* 0x29 */ KEY_BACKQUOTE,
  /* 0x2A */ KEY_SHIFT_L,
  /* 0x2B */ KEY_BACKSLASH,
  /* 0x2C */ KEY_Z,
  /* 0x2D */ KEY_X,
  /* 0x2E */ KEY_C,
  /* 0x2F */ KEY_V,
  /* 0x30 */ KEY_B,
  /* 0x31 */ KEY_N,
  /* 0x32 */ KEY_M,
  /* 0x33 */ KEY_COMMA,
  /* 0x34 */ KEY_POINT,
  /* 0x35 */ KEY_SLASH,
  /* 0x36 */ KEY_SHIFT_R,
  /* 0x37 */ KEY_STAR,
  /* 0x38 */ KEY_ALT_L,
  /* 0x39 */ KEY_SPACE,
  /* 0x3A */ KEY_CAPSLOCK,
  /* 0x3B */ KEY_F1,
  /* 0x3C */ KEY_F2,
  /* 0x3D */ KEY_F3,
  /* 0x3E */ KEY_F4,
  /* 0x3F */ KEY_F5,
  /* 0x40 */ KEY_F6,
  /* 0x41 */ KEY_F7,
  /* 0x42 */ KEY_F8,
  /* 0x43 */ KEY_F9,
  /* 0x44 */ KEY_F10,
  /* 0x45 */ KEY_NUMLOCK,
  /* 0x46 */ KEY_SCRLOCK,
  /* 0x47 */ KEY_PAD_7,
  /* 0x48 */ KEY_PAD_8,
  /* 0x49 */ KEY_PAD_9,
  /* 0x4A */ KEY_PAD_MINUS,
  /* 0x4B */ KEY_PAD_4,
  /* 0x4C */ KEY_PAD_5,
  /* 0x4D */ KEY_PAD_6,
  /* 0x4E */ KEY_PAD_PLUS,
  /* 0x4F */ KEY_PAD_1,
  /* 0x50 */ KEY_PAD_2,
  /* 0x51 */ KEY_PAD_3,
  /* 0x52 */ KEY_PAD_0,
  /* 0x53 */ KEY_PAD_POINT,
  /* 0x54 */ KEY_54,
  /* 0x55 */ KEY_55,
  /* 0x56 */ KEY_56,
  /* 0x57 */ KEY_F11,
  /* 0x58 */ KEY_F12,
  /* 0x59 */ KEY_59,
  /* 0x5A */ KEY_WIN_L,
  /* 0x5B */ KEY_WIN_R,
  /* 0x5C */ KEY_CLIPBOARD,
  /* 0x5D */ KEY_5D,
  /* 0x5E */ KEY_5E,

  // 以下为自定义按键，为和 keymap 索引匹配
  /* 0x5F */ KEY_PRINT_SCREEN,
} KEY;

static char keymap[][4] = {
    /* 扫描码 未与 shift 组合  与 shift 组合 以及相关状态 */
    /* ---------------------------------- */
    /* 0x00 */ {INV, INV, false, false},    // NULL
    /* 0x01 */ {0x1b, 0x1b, false, false},  // ESC
    /* 0x02 */ {'1', '!', false, false},
    /* 0x03 */ {'2', '@', false, false},
    /* 0x04 */ {'3', '#', false, false},
    /* 0x05 */ {'4', '$', false, false},
    /* 0x06 */ {'5', '%', false, false},
    /* 0x07 */ {'6', '^', false, false},
    /* 0x08 */ {'7', '&', false, false},
    /* 0x09 */ {'8', '*', false, false},
    /* 0x0A */ {'9', '(', false, false},
    /* 0x0B */ {'0', ')', false, false},
    /* 0x0C */ {'-', '_', false, false},
    /* 0x0D */ {'=', '+', false, false},
    /* 0x0E */ {'\b', '\b', false, false},  // BACKSPACE
    /* 0x0F */ {'\t', '\t', false, false},  // TAB
    /* 0x10 */ {'q', 'Q', false, false},
    /* 0x11 */ {'w', 'W', false, false},
    /* 0x12 */ {'e', 'E', false, false},
    /* 0x13 */ {'r', 'R', false, false},
    /* 0x14 */ {'t', 'T', false, false},
    /* 0x15 */ {'y', 'Y', false, false},
    /* 0x16 */ {'u', 'U', false, false},
    /* 0x17 */ {'i', 'I', false, false},
    /* 0x18 */ {'o', 'O', false, false},
    /* 0x19 */ {'p', 'P', false, false},
    /* 0x1A */ {'[', '{', false, false},
    /* 0x1B */ {']', '}', false, false},
    /* 0x1C */ {'\n', '\n', false, false},  // ENTER
    /* 0x1D */ {INV, INV, false, false},    // CTRL_L
    /* 0x1E */ {'a', 'A', false, false},
    /* 0x1F */ {'s', 'S', false, false},
    /* 0x20 */ {'d', 'D', false, false},
    /* 0x21 */ {'f', 'F', false, false},
    /* 0x22 */ {'g', 'G', false, false},
    /* 0x23 */ {'h', 'H', false, false},
    /* 0x24 */ {'j', 'J', false, false},
    /* 0x25 */ {'k', 'K', false, false},
    /* 0x26 */ {'l', 'L', false, false},
    /* 0x27 */ {';', ':', false, false},
    /* 0x28 */ {'\'', '"', false, false},
    /* 0x29 */ {'`', '~', false, false},
    /* 0x2A */ {INV, INV, false, false},  // SHIFT_L
    /* 0x2B */ {'\\', '|', false, false},
    /* 0x2C */ {'z', 'Z', false, false},
    /* 0x2D */ {'x', 'X', false, false},
    /* 0x2E */ {'c', 'C', false, false},
    /* 0x2F */ {'v', 'V', false, false},
    /* 0x30 */ {'b', 'B', false, false},
    /* 0x31 */ {'n', 'N', false, false},
    /* 0x32 */ {'m', 'M', false, false},
    /* 0x33 */ {',', '<', false, false},
    /* 0x34 */ {'.', '>', false, false},
    /* 0x35 */ {'/', '?', false, false},
    /* 0x36 */ {INV, INV, false, false},   // SHIFT_R
    /* 0x37 */ {'*', '*', false, false},   // PAD *
    /* 0x38 */ {INV, INV, false, false},   // ALT_L
    /* 0x39 */ {' ', ' ', false, false},   // SPACE
    /* 0x3A */ {INV, INV, false, false},   // CAPSLOCK
    /* 0x3B */ {INV, INV, false, false},   // F1
    /* 0x3C */ {INV, INV, false, false},   // F2
    /* 0x3D */ {INV, INV, false, false},   // F3
    /* 0x3E */ {INV, INV, false, false},   // F4
    /* 0x3F */ {INV, INV, false, false},   // F5
    /* 0x40 */ {INV, INV, false, false},   // F6
    /* 0x41 */ {INV, INV, false, false},   // F7
    /* 0x42 */ {INV, INV, false, false},   // F8
    /* 0x43 */ {INV, INV, false, false},   // F9
    /* 0x44 */ {INV, INV, false, false},   // F10
    /* 0x45 */ {INV, INV, false, false},   // NUMLOCK
    /* 0x46 */ {INV, INV, false, false},   // SCRLOCK
    /* 0x47 */ {'7', INV, false, false},   // pad 7 - Home
    /* 0x48 */ {'8', INV, false, false},   // pad 8 - Up
    /* 0x49 */ {'9', INV, false, false},   // pad 9 - PageUp
    /* 0x4A */ {'-', '-', false, false},   // pad -
    /* 0x4B */ {'4', INV, false, false},   // pad 4 - Left
    /* 0x4C */ {'5', INV, false, false},   // pad 5
    /* 0x4D */ {'6', INV, false, false},   // pad 6 - Right
    /* 0x4E */ {'+', '+', false, false},   // pad +
    /* 0x4F */ {'1', INV, false, false},   // pad 1 - End
    /* 0x50 */ {'2', INV, false, false},   // pad 2 - Down
    /* 0x51 */ {'3', INV, false, false},   // pad 3 - PageDown
    /* 0x52 */ {'0', INV, false, false},   // pad 0 - Insert
    /* 0x53 */ {'.', 0x7F, false, false},  // pad . - Delete
    /* 0x54 */ {INV, INV, false, false},   //
    /* 0x55 */ {INV, INV, false, false},   //
    /* 0x56 */ {INV, INV, false, false},   //
    /* 0x57 */ {INV, INV, false, false},   // F11
    /* 0x58 */ {INV, INV, false, false},   // F12
    /* 0x59 */ {INV, INV, false, false},   //
    /* 0x5A */ {INV, INV, false, false},   //
    /* 0x5B */ {INV, INV, false, false},   // Left Windows
    /* 0x5C */ {INV, INV, false, false},   // Right Windows
    /* 0x5D */ {INV, INV, false, false},   // Clipboard
    /* 0x5E */ {INV, INV, false, false},   //

    // Print Screen 是强制定义 本身是 0xB7
    /* 0x5F */ {INV, INV, false, false},  // PrintScreen
};

static lock_t lock;     // 锁
static task_t *waiter;  // 等待输入的任务

#define BUFFER_SIZE 64         // 输入缓冲区大小
static char buf[BUFFER_SIZE];  // 输入缓冲区
static fifo_t fifo;            // 循环队列

static bool capslock_state;  // 大写锁定
static bool scrlock_state;   // 滚动锁定
static bool numlock_state;   // 数字锁定
static bool extcode_state;   // 扩展码状态

// CTRL 键状态
#define ctrl_state (keymap[KEY_CTRL_L][2] || keymap[KEY_CTRL_L][3])
// ALT 键状态
#define alt_state (keymap[KEY_ALT_L][2] || keymap[KEY_ALT_L][3])
// SHIFT 键状态
#define shift_state (keymap[KEY_SHIFT_L][2] || keymap[KEY_SHIFT_R][2])

static void keyboard_wait()
{
  u8 state;
  do
  {
    state = inb(KEYBOARD_CTRL_PORT);
  } while (state & 0x02);  // 读取键盘缓冲区，直到为空
}

static void keyboard_ack()
{
  u8 state;
  do
  {
    state = inb(KEYBOARD_DATA_PORT);
  } while (state != KEYBOARD_CMD_ACK);
}

static void set_leds()
{
  u8 leds = (capslock_state << 2) | (numlock_state << 1) | scrlock_state;
  keyboard_wait();
  // 设置 LED 命令
  outb(KEYBOARD_DATA_PORT, KEYBOARD_CMD_LED);
  keyboard_ack();

  keyboard_wait();
  // 设置 LED 灯状态
  outb(KEYBOARD_DATA_PORT, leds);
  keyboard_ack();
}

void keyboard_handler(int vector)
{
  assert(vector == 0x21);
  send_eoi(vector);                        // 发送中断处理完成信号
  u16 scancode = inb(KEYBOARD_DATA_PORT);  // 从键盘读取按键信息扫描码
  u8 ext = 2;                              // keymap 状态索引，默认没有 shift 键
  // 扩展码会连续读出两次扫描码
  // 第一次是扩展码字节，比如小键盘的回车键按下
  if (scancode == 0xe0)
  {
    // 置扩展状态
    extcode_state = true;
    // DEBUGK("extend scancode 0x%x\n", scancode);
    return;
  }
  // DEBUGK("scancode 0x%x\n", scancode);
  //  第二次读入的是扫描码
  if (extcode_state)
  {
    // 改状态索引
    ext = 3;
    // 修改扫描码，添加 0xe0 前缀
    scancode |= 0xe000;
    // 扩展状态无效
    extcode_state = false;
  }
  // 获得通码
  u16 makecode = (scancode & 0x7f);
  if (makecode == CODE_PRINT_SCREEN_DOWN)
  {
    makecode = KEY_PRINT_SCREEN;
  }
  // 通码非法
  if (makecode > KEY_PRINT_SCREEN)
  {
    return;
  }
  // DEBUGK("scancode 0x%x\n", scancode);
  // 是否是断码，按键抬起
  bool breakcode = ((scancode & 0x0080) != 0);
  if (breakcode)
  {
    // 如果是则设置状态
    keymap[makecode][ext] = false;
    return;
  }
  // 下面是通码，按键按下
  keymap[makecode][ext] = true;
  // 是否需要设置 LED 灯
  bool led = false;
  if (makecode == KEY_NUMLOCK)
  {
    numlock_state = !numlock_state;
    led = true;
  }
  else if (makecode == KEY_CAPSLOCK)
  {
    capslock_state = !capslock_state;
    led = true;
  }
  else if (makecode == KEY_SCRLOCK)
  {
    scrlock_state = !scrlock_state;
    led = true;
  }

  if (led)
  {
    set_leds();
  }

  // 计算 shift 状态
  bool shift = false;
  if (capslock_state && ('a' <= keymap[makecode][0] <= 'z'))
  {
    shift = !shift;
  }
  if (shift_state)
  {
    shift = !shift;
  }
  // 获得按键 ASCII 码
  char ch = 0;
  // [/?] 这个键比较特殊，只有这个键扩展码和普通码一样
  if (ext == 3 && (makecode != KEY_SLASH))
  {
    // DEBUGK("scancode 0x%x\n", scancode);
    ch = keymap[makecode][1];  // ? 号
  }
  else
  {
    ch = keymap[makecode][shift];
  }
  if (ch == INV)
    return;
  fifo_put(&fifo, ch);
  if (waiter != NULL)  // 有等待输入的进程
  {
    task_unblock(waiter);  // 解除阻塞
    waiter = NULL;
  }
}

u32 keyboard_read(char *buf, u32 count)
{
  lock_acquire(&lock);
  int nr = 0;
  while (nr < count)
  {
    while (fifo_empty(&fifo))
    {
      waiter = running_task();
      task_block(waiter, NULL, TASK_WAITING);
    }
    buf[nr++] = fifo_get(&fifo);
  }
  lock_release(&lock);
  return count;
}

void keyboard_init()
{
  numlock_state = false;
  scrlock_state = false;
  capslock_state = false;
  extcode_state = false;

  fifo_init(&fifo, buf, BUFFER_SIZE);
  lock_init(&lock);
  waiter = NULL;

  set_leds();
  set_interrupt_handler(IRQ_KEYBOARD, keyboard_handler);
  set_interrupt_mask(IRQ_KEYBOARD, true);
}
