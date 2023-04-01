#include <neonix/debug.h>
#include <neonix/interrupt.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern void memory_map_init();
extern void mapping_init();
extern void arena_init();
extern void interrupt_init();
extern void clock_init();
extern void time_init();
extern void rtc_init();
extern void keyboard_init();
extern void ide_init();
extern void task_init();
extern void syscall_init();
extern void hang();
extern void tss_init();
extern void buffer_init();
extern void file_init();
extern void super_init();
extern void inode_init();

void kernel_init()
{
  tss_init();
  memory_map_init();
  mapping_init();
  arena_init();
  interrupt_init();
  clock_init();
  keyboard_init();
  time_init();
  // rtc_init();
  ide_init();
  syscall_init();
  task_init();
  buffer_init();
  file_init();
  inode_init();
  super_init();
  set_interrupt_state(true);
}
