#include "sync.h"
#include "stdint.h"
#include "deque.h"
#include "interrupt.h"
#include "thread.h"
#include "debug.h"

#define NULL ((void*)0)

void sema_init(struct semaphore* sema, uint8_t value)
{
    sema->value = value;
    deque_init(&sema->waiters);
}

void lock_init(struct lock* lck)
{
    lck->holder = NULL;
    lck->holder_repeat_nr = 0;
    sema_init(&lck->semaphore, 1);
}

void sema_down(struct semaphore* sema)
{
    enum intr_status old_status = intr_disable();
    while(sema->value == 0)                                                        //线程被唤醒后还需要做判断, 确保当前信号量确实不为0
    {
        struct PCB* cur = running_thread();
        ASSERT(!elem_find(&sema->waiters, &cur->general_tag));
        if(elem_find(&sema->waiters, &cur->general_tag))
        {
            PANIC("sema_down: thread blocked has been in waiters_list\n");
        }
        deque_append(&sema->waiters, &cur->general_tag);
        thread_block(TASK_BLOCKED);
    }
    sema->value--;
    ASSERT(sema->value == 0);
    intr_set_status(old_status);
}

void sema_up(struct semaphore* sema)
{
    enum intr_status old_status = intr_disable();
    ASSERT(sema->value == 0);
    if(!deque_empty(&sema->waiters))
    {
        struct PCB* thread_blocked = elem_to_entry(struct PCB, general_tag, deque_pop(&sema->waiters));
        thread_unblock(thread_blocked);
    }
    sema->value++;
    ASSERT(sema->value == 1);
    intr_set_status(old_status);
}

void lock_acquire(struct lock* lck)
{
    if(lck->holder != running_thread())
    {
        sema_down(&lck->semaphore);
        lck->holder = running_thread();
        ASSERT(lck->holder_repeat_nr == 0);
        lck->holder_repeat_nr = 1;
    }
    else 
    {
        lck->holder_repeat_nr++;
    }
}

void lock_release(struct lock* lck)
{
    ASSERT(lck->holder == running_thread());
    if(lck->holder_repeat_nr > 1)
    {
        lck->holder_repeat_nr--;
        return;
    }
    ASSERT(lck->holder_repeat_nr == 1);
    lck->holder = NULL;
    lck->holder_repeat_nr = 0;
    sema_up(&lck->semaphore);
}