#include "print.h"
#include "sync.h"
#include "stdint.h"
#include "thread.h"
#include "interrupt.h"

void console_init();
void console_acquire();
void console_release();
void console_put_str(char* str);
void console_put_char(uint8_t ch);
void console_put_int(uint32_t num);