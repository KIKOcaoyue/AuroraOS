#include "syscallinit.h"
#include "memory.h"
#include "fs.h"

pid_t sys_getpid()
{
    return running_thread()->pid;
}

void syscall_init()
{
    put_str("syscall init start\n");
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_WRITE] = sys_write;
    syscall_table[SYS_MALLOC] = sys_malloc;
    syscall_table[SYS_FREE] = sys_free;
    put_str("syscall init done\n");
}