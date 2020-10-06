#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

/* 初始化位图btmp */
void bitmap_init(struct bitmap* btmp)
{
    memset(btmp->bits, 0, btmp->bytelen);
}

/* 判断bit_idx位是否为1, 若为1, 则返回ture, 否则返回false */
int32_t bitmap_bit_check(struct bitmap* btmp, uint32_t bit_idx)
{
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_lef  = bit_idx % 8;
    return (btmp->bits[byte_idx]) & (BITMAP_MASK << bit_lef);
}

/* 在位图中申请连续的cnt位, 返回起始位下标, 否则返回-1 */
int bitmap_con_check(struct bitmap* btmp, uint32_t cnt)
{
    uint32_t byte_idx = 0;
    while((btmp->bits[byte_idx] == 0xff) && (byte_idx < btmp->bytelen))
    {
        byte_idx++;
    }
    //ASSERT(byte_idx < btmp->bytelen);
    if(byte_idx >= btmp->bytelen)
    {
        return -1;
    }
    int bit_idx = 0;
    while(btmp->bits[byte_idx] & (uint8_t)(BITMAP_MASK << bit_idx))
    {
        bit_idx++;
    }
    int start = byte_idx * 8 + bit_idx;
    if(cnt==1)
    {
        return start;
    }
    uint32_t bit_left = btmp->bytelen * 8 - start;
    uint32_t nxt = start + 1;
    uint32_t count = 1;
    start = -1;
    while(bit_left-- > 0)
    {
        if(!bitmap_bit_check(btmp,nxt))
        {
            count++;
        }
        else
        {
            count = 0;
        }
        if(count==cnt)
        {
            start = nxt - cnt + 1;
            break;
        }
        nxt++;
    }
    return start;
}

/* 将位图btmp的bit_idx位设置为value */
void bitmap_setbit(struct bitmap* btmp, uint32_t bit_idx, int8_t value)
{
    ASSERT((value==0) || (value==1));
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_lef  = bit_idx % 8;
    if(value)
    {
        btmp->bits[byte_idx] |= (BITMAP_MASK << bit_lef);
    }
    else
    {
        btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_lef);
    }
}
