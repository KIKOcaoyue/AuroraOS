#include "syscall.h"

uint32_t getpid()
{
    return _syscall0(SYS_GETPID);
}


void* malloc(uint32_t size)
{
    return (void*)_syscall1(SYS_MALLOC, size);
}

void free(void* ptr)
{
    _syscall1(SYS_FREE, ptr);
}

uint32_t write(int32_t fd, const void* buf, uint32_t count)
{
    return _syscall3(SYS_WRITE, fd, buf, count);
}