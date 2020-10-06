#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include "deque.h"
#include "stdint.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"

struct semaphore
{
    uint8_t      value;
    struct DEQUE waiters;
};

struct lock
{
    struct PCB*      holder;           
    struct semaphore semaphore;             //二元信号量
    uint32_t         holder_repeat_nr;      //锁的持有者重复申请锁的次数
};

void sema_init(struct semaphore* sema, uint8_t value);
void lock_init(struct lock* lck);
void sema_down(struct semaphore* sema);
void sema_up(struct semaphore* sema);
void lock_acquire(struct lock* lck);
void lock_release(struct lock* lck);




#endif 