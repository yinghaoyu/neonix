#ifndef NEONIX_TYPES_H
#define NEONIX_TYPES_H
#include <neonix/neonix.h>

#define EOF -1  // 文件结尾

#define NULL ((void *) 0)  // 空指针

#define EOS '\0'  // 字符串结尾

#define CONCAT(x, y) x##y
#define RESERVED_TOKEN(x, y) CONCAT(x, y)
#define RESERVED RESERVED_TOKEN(reserved, __LINE__)

#ifndef __cplusplus  // 兼容c++, 否则以下宏定义会出错
#define bool _Bool
#define true 1
#define false 0
#endif

// 用于取消结构体内存对齐
#define _packed __attribute__((packed))

// 用于省略函数的栈帧
#define _ofp __attribute__((optimize("omit-frame-pointer")))

// 给编译器参考, 函数不一定展开
#define _inline __attribute__((always_inline)) inline

typedef unsigned int size_t;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef int32 pid_t;
typedef int32 dev_t;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef u32 time_t;
typedef u32 idx_t;

typedef u16 mode_t; // 文件权限

typedef int32 fd_t;
typedef enum std_fd_t
{
  stdin,
  stdout,
  stderr,
} std_fd_t;

#endif
