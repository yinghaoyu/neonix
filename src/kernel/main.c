#include <neonix/debug.h>
#include <neonix/interrupt.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern void memory_map_init();
extern void mapping_init();
extern void interrupt_init();
extern void clock_init();
extern void time_init();
extern void rtc_init();
extern void keyboard_init();
extern void task_init();
extern void syscall_init();
extern void hang();
extern void tss_init();

void kernel_init()
{
  tss_init();
  memory_map_init();
  mapping_init();
  interrupt_init();
  clock_init();
  keyboard_init();
  // time_init();
  // rtc_init();
  task_init();
  syscall_init();
  set_interrupt_state(true);
  return;
}
