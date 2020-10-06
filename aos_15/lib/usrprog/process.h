#ifndef __USRPROG_PROCESS_H
#define __USRPROG_PROCESS_H

#include "bitmap.h"
#include "thread.h"
#include "memory.h"
#include "global.h"
#include "debug.h"
#include "stdint.h"
#include "deque.h"
#include "interrupt.h"

#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define USER_VADDR_START  0x8048000
#define DEFAULT_PRIO      31

extern void intr_exit();

void start_process(void* filename_);

void page_dir_activate(struct PCB* pthread);

void process_activate(struct PCB* pthread);

/* 创建用户进程的页目录表, 并且返回起始虚拟地址 */
uint32_t* create_page_dir();

/* 创建虚拟内存池 */
void create_user_vaddr_bitmap(struct PCB* user_prog);


/* 创建用户进程并且加入就绪队列 */
void process_execute(void* filename, char* name);

#endif 