#include "io.h"
#include "print.h"
#include "stdint.h"
#include "interrupt.h"

static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl, uint8_t counter_mode, uint16_t counter_value);

static void intr_timer_handler();

void timer_init();
