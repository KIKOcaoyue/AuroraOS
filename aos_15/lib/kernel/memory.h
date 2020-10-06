#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
#include "deque.h"


//PG前缀表示页表项或页目录项, US表示第2位的US位, RW表示第1位的RW位, P表示第0位的P位
#define PG_P_1          1           //此页内存 存在
#define PG_P_0          0           //此页内存 不存在
#define PG_RW_R         0           //R/W属性位值, 读/执行 
#define PG_RW_W         2           //R/W属性位值, 读/写/执行
#define PG_US_S         0           //U/S属性位值, 系统级
#define PG_US_U         4           //U/S属性位值, 用户级

#define PAGE_SIZE       4096                 //页的大下

#define MEM_BITMAP_BASE 0xc009a000           //位图基地址 ()

#define K_HEAP_START    0xc0100000           //内核堆地址 (0xc0000000是内核从虚拟地址3G起, 0x100000意指跨过低端1MB内存, 使虚拟地址在逻辑上连续)

#define PDE_IDX(addr)   ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr)   ((addr & 0x003ff000) >> 12)

#define DESC_CNT        7

enum pool_flags
{
    PF_KERNEL = 1,
    PF_USER   = 2
};

struct virtual_addr
{
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

struct mem_block
{
    struct DEQUE_ELEM free_elem;
};

struct mem_block_desc
{
    uint32_t block_size;
    uint32_t blocks_per_arena;
    struct DEQUE free_list;
};

extern struct pool kernel_pool, user_pool;
void mem_init();

void block_desc_init(struct mem_block_desc* desc_array);

/* 在堆中申请size字节内存 */
void* sys_malloc(uint32_t size);

void sys_free(void* ptr);

#endif 