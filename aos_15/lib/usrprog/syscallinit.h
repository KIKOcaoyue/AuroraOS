#ifndef __USRPROG_SYSCALLINIT_H
#define __USRPROG_SYSCALLINIT_H
#include "stdint.h"
#include "thread.h"
#include "string.h"
#include "syscall.h"
#include "memory.h"

#define syscall_nr 32


typedef void* syscall;
syscall syscall_table[syscall_nr];

pid_t sys_getpid();
uint32_t write(int32_t fd, const void* buf, uint32_t count);
void syscall_init();
#endif 