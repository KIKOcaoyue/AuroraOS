#include "iobuf.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"
#include "thread.h"


void iobuf_init(struct iobuf* iob)
{
    lock_init(&iob->lck);
    iob->producer = iob->consumer = NULL;
    iob->head = iob->tail = 0;
}

static int32_t next_pos(int32_t pos)
{
    return (pos+1)%bufsize;
}

int iob_full(struct iobuf* iob)
{
    ASSERT(intr_get_status() == INTR_OFF);
    return next_pos(iob->head)==iob->tail;
}

int iob_empty(struct iobuf* iob)
{
    ASSERT(intr_get_status() == INTR_OFF);
    return iob->head == iob->tail;
}

static void iob_wait(struct PCB** waiter)
{
    ASSERT(*waiter==NULL && waiter != NULL);
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}

static void wakeup(struct PCB** waiter)
{
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter);
    *waiter = NULL;
}

char iob_getchar(struct iobuf* iob)
{
    ASSERT(intr_get_status()==INTR_OFF);
    while(iob_empty(iob))
    {
        lock_acquire(&iob->lck);
        iob_wait(&iob->consumer);
        lock_release(&iob->lck);
    }
    char byte = iob->buf[iob->tail];
    iob->tail = next_pos(iob->tail);
    if(iob->producer != NULL)
    {
        wakeup(&iob->producer);
    }
    return byte;
}

void iob_putchar(struct iobuf* iob, char byte)
{
    ASSERT(intr_get_status()==INTR_OFF);
    while(iob_full(iob))
    {
        lock_acquire(&iob->lck);
        iob_wait(&iob->producer);
        lock_release(&iob->lck);
    }
    iob->buf[iob->head] = byte;
    iob->head = next_pos(iob->head);
    if(iob->consumer != NULL)
    {
        wakeup(&iob->consumer);
    }
}
