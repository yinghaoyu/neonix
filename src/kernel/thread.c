#include <neonix/debug.h>
#include <neonix/interrupt.h>
#include <neonix/syscall.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

void idle_thread()
{
  set_interrupt_state(true);
  u32 counter = 0;
  while (true)
  {
    // LOGK("idle task.... %d\n", counter++);
    asm volatile(
        "sti\n"  // 开中断
        "hlt\n"  // 关闭 CPU，进入暂停状态，等待外中断的到来
    );
    yield();  // 放弃执行权，调度执行其他任务
  }
}

#include <neonix/mutex.h>

// mutex_t mutex;
lock_t lock;

void init_thread()
{
  // mutex_init(&mutex);
  lock_init(&lock);
  set_interrupt_state(true);
  u32 counter = 0;

  char ch;

  while (true)
  {
    bool intr = interrupt_disable();
    keyboard_read(&ch, 1);
    printk("%c", ch);
    set_interrupt_state(intr);
    // mutex_lock(&mutex);
    // lock_acquire(&lock);
    // LOGK("init task %d....\n", counter++);
    // lock_release(&lock);
    // mutex_unlock(&mutex);
    // sleep(500);
  }
}

void test_thread()
{
  set_interrupt_state(true);
  u32 counter = 0;

  while (true)
  {
    // mutex_lock(&mutex);
    // lock_acquire(&lock);
    // LOGK("test task %d....\n", counter++);
    // lock_release(&lock);
    // mutex_unlock(&mutex);
    sleep(709);
  }
}
