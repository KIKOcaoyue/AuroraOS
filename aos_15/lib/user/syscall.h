#ifndef __USER_SYSCALL_H
#define __USER_SYSCALL_H
#include "stdint.h"

//retval在最后是一个独立的语句, 在宏中这样写可以直接返回其值, 与函数中(return retval;)完全一致
#define _syscall0(NUMBER) ({int retval; asm volatile("int $0x80" : "=a" (retval) : "a" (NUMBER) : "memory"); retval;})
#define _syscall1(NUMBER, ARG1) ({int retval; asm volatile("int $0x80" : "=a" (retval) : "a" (NUMBER), "b" (ARG1) : "memory"); retval;})
#define _syscall2(NUMBER, ARG1, ARG2) ({int retval; asm volatile("int $0x80" : "=a" (retval) : "a" (NUMBER), "b" (ARG1), "c" (ARG2) : "memory"); retval;})
#define _syscall3(NUMBER, ARG1, ARG2, ARG3) ({int retval; asm volatile("int $0x80" : "=a" (retval) : "a" (NUMBER), "b" (ARG1), "c" (ARG2), "d" (ARG3) : "memory"); retval;})

enum SYSCALL_NR
{
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE
};
uint32_t getpid();
uint32_t write(int32_t fd, const void* buf, uint32_t count);
void* malloc(uint32_t size);
void free(void* ptr);
#endif 