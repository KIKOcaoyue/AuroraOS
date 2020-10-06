#ifndef __LIB_KERNEL_STRING_H
#define __LIB_KERNEL_STRING_H

#include "print.h"
#include "stdint.h"

#define NULL ((void*)0)

void memset(void* dst_, uint8_t value, uint32_t size);
void memcpy(void* dst_, void* src_, uint32_t size);
int8_t memcmp(const void* a_, const void* b_, uint32_t size);
char* strcpy(char* dst_, char* src_);
uint32_t strlen(const char* str);
int8_t strcmp(const char* a_, const char* b_);
char* strchr(const char* str, const uint8_t ch);
char* strrchr(const char* str, const uint8_t ch);
char* strcat(char* dst_, char* src_);
uint32_t strchrs(const char* str, uint8_t ch);

#endif 