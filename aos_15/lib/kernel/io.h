#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"

/* 向端口port写入一个字节 */
static inline void outb(uint16_t port, uint8_t data)
{
    /*  
        对端口指定N表示0~255, d表示用dx存储端口号, %b0对应al, %w1对应dx  
    */
    asm volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

/* 将addr处起始的word_cnt个字写入端口port */
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt)
{
    /*  
        +表示此限制既作输入, 又作输出
        outsw是把ds:esi处的16位的内容写入port端口
    */
    asm volatile("cld; rep outsw" : "+S" (addr), "+c" (word_cnt) : "d" (port));
}

/* 从端口port读入的一个字节 */
static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

/* 从端口port读入的word_cnt个字写入addr */
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt)
{
    /*
        insw是将从端口port处读入的16位内容写入es:edi指向的内容
    */
    asm volatile("cld; rep insw" : "+D" (addr), "+c" (word_cnt) : "d" (port) : "memory");
}

#endif
