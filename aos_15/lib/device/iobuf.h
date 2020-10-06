#ifndef __DEVICE_IOBUF_H
#define __DEVICE_IOBUF_H
#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define NULL ((void*)0)

#define bufsize 64 


struct iobuf
{
    struct lock lck;
    struct PCB* producer;
    struct PCB* consumer;
    char buf[bufsize];
    int32_t head;
    int32_t tail;
};

void iobuf_init(struct iobuf* iob);

static int32_t next_pos(int32_t pos);

int iob_full(struct iobuf* iob);

int iob_empty(struct iobuf* iob);

static void iob_wait(struct PCB** waiter);

static void wakeup(struct PCB** waiter);

char iob_getchar(struct iobuf* iob);

void iob_putchar(struct iobuf* iob, char byte);


#endif 