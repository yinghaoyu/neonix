#ifndef NEONIX_CONSOLE_H
#define NEONIX_CONSOLE_H

#include <neonix/types.h>

void console_init();
void console_clear();
void console_write(char* buf, u32 count);

#endif
