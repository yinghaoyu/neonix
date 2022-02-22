#ifndef NEONIX_STDIO_H
#define NEONIX_STDIO_H

#include <neonix/stdarg.h>

int vsprintf(char* buf, const char* fmt, va_list args);
int sprintf(char* buf, const char* fmt, ...);

#endif
