#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "memory.h"
#include "timer.h"
#include "thread.h"
#include "sync.h"
#include "console.h"
#include "keyboard.h"
#include "tss.h"
#include "syscall.h"
#include "syscallinit.h"
#include "ide.h"
#include "fs.h"

void init_all()
{
    put_str("init_all\n");
    idt_init();
    mem_init();
    timer_init();
    thread_init();
    console_init();
    keyboard_init();
    tss_init();
    syscall_init();
    ide_init();
    filesys_init();
}