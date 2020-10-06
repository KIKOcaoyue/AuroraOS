#include "string.h"
#include "global.h"
#include "debug.h"

/* 将dst_起始的size个字节置为value */
void memset(void* dst_, uint8_t value, uint32_t size)
{
    ASSERT(dst_!=NULL);
    uint8_t* dst = (uint8_t*) dst_;
    while(size-- > 0)
    {
        *dst++ = value;
    }
}

/* 将src_起始的size个字节复制到dst_ */
void memcpy(void* dst_, void* src_, uint32_t size)
{
    ASSERT(dst_!=NULL && src_!=NULL);
    uint8_t* dst = (uint8_t*) dst_;
    const uint8_t* src = (uint8_t*) src_;
    while(size-- > 0)
    {
        *dst++ = *src++;
    }
}

/* 连续比较以地址a_和地址b_开头的size个字节, 若相等则返回0, 若a_大于b_则返回1, 否则返回-1 */
int8_t memcmp(const void* a_, const void* b_, uint32_t size)
{
    ASSERT(a_!=NULL && b_!=NULL);
    const char* a = a_;
    const char* b = b_;
    while(size-- > 0)
    {
        if(*a!=*b)
        {
            return *a > *b ? 1 : -1;
        }
        a++;
        b++;
    }
}

/* 将字符串从src_复制到dst_. 返回目的字符串的起始地址 */
char* strcpy(char* dst_, char* src_)
{
    ASSERT(dst_!=NULL && src_!=NULL);
    char* r = dst_;
    while((*dst_++ = *src_++));
    return r;
}

/* 返回字符串的长度 */
uint32_t strlen(const char* str)
{
    ASSERT(str!=NULL);
    const char* p = str;
    while(*p++);
    return (p-str-1);
}

/* 比较两个字符串,若a_中的字符大于b_则返回1, 小于返回-1, 相等返回0 */
int8_t strcmp(const char* a_, const char* b_)
{
    ASSERT(a_!=NULL && b_!=NULL);
    while(*a_!=0 && *a_ == *b_)
    {
        a_++;
        b_++;
    }
    return *a_ < *b_ ? -1 : *a_ > *b_;
}

/* 从左向右查找字符串str中首次出现字符ch的地址 */
char* strchr(const char* str, const uint8_t ch)
{
    ASSERT(str!=NULL);
    while(*str!=0)
    {
        if(*str==ch)
        {
            return (char*) str;
        }
        str++;
    }
    return NULL;
}

/* 从后往前查找字符串str中首次字符ch的地址 */
char* strrchr(const char* str, const uint8_t ch)
{
    ASSERT(str!=NULL);
    const char* lst = NULL;
    while(*str!=0)
    {
        if(*str==ch)
        {
            lst = str;
        }
        str++;
    }
    return (char*) lst;
}

/* 将字符串src_拼接到dst_后, 返回拼接的串地址 */
char* strcat(char* dst_, char* src_)
{
    ASSERT(dst_!=NULL && src_!=NULL);
    char* str = dst_;
    while(*str++);
    --str;
    while((*str++ = *src_++));
    return dst_;
}

/* 在字符串str中查找ch出现的次数 */
uint32_t strchrs(const char* str, uint8_t ch)
{
    ASSERT(str!=NULL);
    uint32_t cnt = 0;
    const char* p = str;
    while(*p!=0)
    {
        if(*p==ch)
        {
            cnt++;
        }
        p++;
    }
    return cnt;
}