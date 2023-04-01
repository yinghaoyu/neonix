#ifndef NEONIX_STDARG_H
#define NEONIX_STDARG_H

typedef char *va_list;

// v为第一个参数，因为参数压栈是从右向左，栈地址向下增长，所以v地址加上地址长度后变成下一个参数地址
#define va_start(ap, v) (ap = (va_list) &v + sizeof(char *))
#define va_arg(ap, t) (*(t *) ((ap += sizeof(char *)) - sizeof(char *)))  // 取当前地址的值,并让ap指针指向下一个参数
#define va_end(ap) (ap = (va_list) 0)

#endif
