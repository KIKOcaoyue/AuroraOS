#include "timer.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#include "interrupt.h"
#include "thread.h"
#include "debug.h"
 
#define IRQ0_FREQUENCY       100
#define INPUT_FREQUENCY      1193180
#define COUNTER0_VALUE       INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT        0x40 
#define COUNTER0_NO          0
#define COUNTER_MODE         2
#define READ_WRITE_LATCH     3
#define PIT_CONTROL_PORT     0x43 

#define mil_seconds_per_intr (1000 / IRQ0_FREQUENCY)

uint32_t ticks;

/* 让任务休眠sleep_ticks个ticks */
static void ticks_to_sleep(uint32_t sleep_ticks)
{
    uint32_t start_tick = ticks;
    while(ticks - start_tick < sleep_ticks)
    {
        thread_yield();
    }
}

/* 让程序休眠m_secondes毫秒 */
void mtime_sleep(uint32_t m_seconds)
{
    uint32_t sleep_ticks = DIV_ROUND_UP(m_seconds, mil_seconds_per_intr);
    ASSERT(sleep_ticks > 0);
    ticks_to_sleep(sleep_ticks);
}

static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl, uint8_t counter_mode, uint16_t counter_value)
{
    outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    outb(counter_port, (uint8_t)counter_value);
    outb(counter_port, (uint8_t)counter_value >> 8);
}

static void intr_timer_handler()
{
    struct PCB* cur_thread = running_thread();
    // put_str("\n\n\n\n!!!!!IN timer.c intr_timer_handler()!!!!!\n");
    // put_str("stack_magic = 0x");
    // put_int(cur_thread->stack_magic);
    // put_str("\n");
    ASSERT(cur_thread->stack_magic==0x20200620);
    cur_thread->elapsed_ticks++;
    ticks++;
    if(cur_thread->ticks==0)
    {
        schedule();
    }
    else
    {
        cur_thread->ticks--;   
    }
}

void timer_init()
{
    put_str("timer_init start\n");
    frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    register_handler(0x20, intr_timer_handler);
    put_str("timer_init done\n");
}


