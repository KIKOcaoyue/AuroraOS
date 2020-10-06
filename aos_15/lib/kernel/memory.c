#include "memory.h"
#include "thread.h"
#include "print.h"
#include "global.h"
#include "string.h"
#include "sync.h"

struct pool
{
    struct bitmap pool_bitmap;
    uint32_t      phy_addr_start;
    uint32_t      pool_size;
    struct lock   lck;
};

struct pool kernel_pool, user_pool;

struct virtual_addr kernel_vaddr;

struct arena
{
    struct mem_block_desc* desc;
    uint32_t cnt;
    int large;                           //large == 1时cnt表示页框数, large == 0时cnt表示mem_block数量
};

struct mem_block_desc k_block_descs[DESC_CNT];

static void mem_pool_init(uint32_t all_mem)
{
    put_str("mem_pool_init start\n");
    uint32_t page_table_size        = PAGE_SIZE * 256;                           //页目录表+页表 (页目录表中, 0和768指向第一个页表, 769~1022指向254个页表, 1023指向自己, 不算) 故页目录表(1个)+(页表255个) = 256
    uint32_t userd_mem              = page_table_size + 0x100000;                //已使用的内存(低端1M+页目录表+页表)
    uint32_t free_mem               = all_mem - userd_mem;
    uint16_t all_free_page          = free_mem / PAGE_SIZE;
    uint16_t kernel_free_page       = all_free_page / 2;
    uint16_t user_free_page         = all_free_page - kernel_free_page;
    uint32_t kbm_len                = kernel_free_page / 8;                      //kernel bitmap的长度(字节), 位图中的一位表示一页
    uint32_t ubm_len                = user_free_page / 8;                        //user bitmap的长度     kernel和user均丢弃余数, 简化操作
    uint32_t kp_start               = userd_mem;                                 //内核内存池的起始地址 
    uint32_t up_start               = kp_start + kernel_free_page * PAGE_SIZE;   //用户内存池的起始地址

    kernel_pool.phy_addr_start      = kp_start;
    user_pool.phy_addr_start        = up_start;
    kernel_pool.pool_size           = kernel_free_page * PAGE_SIZE;
    user_pool.pool_size             = user_free_page * PAGE_SIZE;
    kernel_pool.pool_bitmap.bytelen = kbm_len;
    user_pool.pool_bitmap.bytelen   = ubm_len;

    kernel_pool.pool_bitmap.bits    = (void*) MEM_BITMAP_BASE;
    user_pool.pool_bitmap.bits      = (void*) (MEM_BITMAP_BASE + kbm_len);

    put_str("kernel_pool_bitmap_start: ");
    put_int((int)kernel_pool.pool_bitmap.bits);
    put_char('\n');
    put_str("kernel_pool_phy_addr_start: ");
    put_int(kernel_pool.phy_addr_start);
    put_char('\n');

    put_str("user_pool_bitmap_start: ");
    put_int((int)user_pool.pool_bitmap.bits);
    put_char('\n');
    put_str("user_pool_phy_addr_start: ");
    put_int(user_pool.phy_addr_start);
    put_char('\n');

    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    kernel_vaddr.vaddr_bitmap.bytelen = kbm_len;
    kernel_vaddr.vaddr_bitmap.bits = (void*) (MEM_BITMAP_BASE + kbm_len + ubm_len);
    kernel_vaddr.vaddr_start = K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    lock_init(&kernel_pool.lck);
    lock_init(&user_pool.lck);

    put_str("mem_pool_init done\n");
}

/* 在pool_flags表示的虚拟内存池中申请cnt个虚拟页 */
/* 返回虚拟页起始地址或者NULL */
static void* vaddr_get(enum pool_flags pf, uint32_t cnt)
{
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t tmp = 0;
    if(pf==PF_KERNEL)
    {
        bit_idx_start = bitmap_con_check(&kernel_vaddr.vaddr_bitmap, cnt);
        if(bit_idx_start == -1)
        {
            return NULL;
        }
        while(tmp < cnt)
        {
            bitmap_setbit(&kernel_vaddr.vaddr_bitmap, bit_idx_start + tmp++, 1);
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PAGE_SIZE;
    }
    else
    {
        struct PCB* cur = running_thread();
        bit_idx_start = bitmap_con_check(&cur->usrprog_vaddr.vaddr_bitmap, cnt);
        if(bit_idx_start == -1)
        {
            return NULL;
        }
        while(tmp < cnt)
        {
            bitmap_setbit(&cur->usrprog_vaddr.vaddr_bitmap, bit_idx_start + tmp++, 1);
        }
        vaddr_start = cur->usrprog_vaddr.vaddr_start + bit_idx_start * PAGE_SIZE;
        ASSERT((uint32_t)vaddr_start < (0xc0000000 - PAGE_SIZE));
    }
    return (void*)vaddr_start;
}

/* 得到虚拟地址vaddr对应的PTE指针 */
/*
    由于在分页模式下, 处理器访问内存通过虚拟内存的形式访问, 其访问过程分为3步:
    (1) 将虚拟地址的高10位作为页目录表中的索引, 取出其pde指向的页表的物理地址
    (2) 将(1)中的页表物理地址作为基地址, 将虚拟地址的中间10位作为页表中的索引, 取出其pte指向的物理页的物理地址
    (3) 将虚拟地址最后12位作为物理页中的偏移, 访问目标物理地址

    由于处理器根据虚拟地址直接定位到物理页的物理地址, 但是现在只是想取到页表中pte的物理地址
    其方法是: 让处理器认为页目录表是页表
    
    由于页目录表最后一个pde指向的是页目录表自己的物理地址, 于是首先访问页目录表的第1023个pde, 处理器以为现在已经拿到页表的物理地址了, 可实际上还是页目录表的物理地址, 再经过后续两个步骤, 就可以取出pte的物理地址
    
*/
uint32_t* pte_ptr(uint32_t vaddr)
{
    uint32_t* pte = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4); //1023的十六进制为0x3ff, 将其移到高10位后, 变成0xffc00000. PIE_IDX * 4构造出虚拟地址的低12位, 用来在页表中访问到pte
    return pte;
}

/* 得到虚拟地址vaddr对应的PDE指针 */
uint32_t* pde_ptr(uint32_t vaddr)
{
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
}

/* 在m_pool指向的物理内存池中分配1个物理页 */
/* 返回页框的物理地址或NULL */
static void* palloc(struct pool* m_pool)
{
    int bit_idx = bitmap_con_check(&m_pool->pool_bitmap, 1);
    if(bit_idx==-1)
    {
        return NULL;
    }
    bitmap_setbit(&m_pool->pool_bitmap, bit_idx, 1);
    uint32_t page_phyaddr = ((bit_idx * PAGE_SIZE) + m_pool->phy_addr_start);
    return (void*)page_phyaddr;
}

/* 页表中添加虚拟地址_vaddr到物理地址_page_phyaddr的映射 */
static void page_table_add(void* _vaddr, void* _page_phyaddr)
{
    uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t* pde  = pde_ptr(vaddr);
    uint32_t* pte  = pte_ptr(vaddr);
    
    if(*pde & 0x00000001)
    {
        ASSERT(!(*pte & 0x00000001));
        if(!(*pte & 0x00000001))
        {
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
        else
        {
            PANIC("pte repeat");
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
    }
    else                                                                //页目录项不存在, 先创建
    {
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);          //创建一个页表
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        memset((void*)((int)pte & 0xfffff000), 0, PAGE_SIZE);
        ASSERT(!(*pte & 0x00000001));
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

/* 分配cnt个页空间, 返回起始虚拟地址或NULL */
/*
    (1) 首先通过vaddr_get在虚拟内存中申请虚拟内存
    (2) 其次通过palloc在物理内存池中申请物理页
    (3) 最后通过page_table_add将以上得到的虚拟地址和物理地址在页表中映射
*/
void* malloc_page(enum pool_flags pf, uint32_t cnt)
{
    ASSERT(cnt > 0 && cnt < 3840);
    void* vaddr_start = vaddr_get(pf, cnt);
    if(vaddr_start==NULL)
    {
        return NULL;
    }
    uint32_t vaddr = (uint32_t)vaddr_start, tmp = cnt;
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    while(tmp-- > 0)
    {
        void* page_phyaddr = palloc(mem_pool);
        if(page_phyaddr==NULL)
        {
            //TODO 内存回收
            //失败了要将虚拟内存和物理页全部回滚
            return NULL;
        }
        page_table_add((void*)vaddr, page_phyaddr);
        vaddr += PAGE_SIZE;
    }
    return vaddr_start;
}


/* 从内核物理内存池中申请cnt页内存, 返回虚拟地址或NULL */
void* get_kernel_pages(uint32_t cnt)
{
    void* vaddr = malloc_page(PF_KERNEL,cnt);
    if(vaddr!=NULL)
    {
        memset(vaddr, 0, PAGE_SIZE * cnt);
    }
    return vaddr;
}

void* get_user_pages(uint32_t cnt)
{
    lock_acquire(&user_pool.lck);
    void* vaddr = malloc_page(PF_USER, cnt);
    memset(vaddr, 0, PAGE_SIZE * cnt);
    lock_release(&user_pool.lck);
    return vaddr;
}

void* get_a_page(enum pool_flags pf, uint32_t vaddr)
{
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lck);
    struct PCB* cur = running_thread();
    int32_t bit_idx = -1;
    if(cur->pgdir != NULL && pf == PF_USER)
    {
        bit_idx = (vaddr - cur->usrprog_vaddr.vaddr_start) / PAGE_SIZE;
        ASSERT(bit_idx > 0);
        bitmap_setbit(&cur->usrprog_vaddr.vaddr_bitmap, bit_idx, 1);
    }
    else if(cur->pgdir == NULL && pf == PF_KERNEL)
    {
        bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PAGE_SIZE;
        ASSERT(bit_idx > 0);
        bitmap_setbit(&kernel_vaddr.vaddr_bitmap, bit_idx, 1);
    }
    else
    {
        PANIC("get_a_page: not allow kernel alloc userspace or user alloc kernelspace by get_a_page\n");
    }
    void* page_phyaddr = palloc(mem_pool);
    if(page_phyaddr == NULL)
    {
        return NULL;
    }
    page_table_add((void*)vaddr, page_phyaddr);
    lock_release(&mem_pool->lck);
    return (void*)vaddr;
}

/* 将虚拟地址映射到物理地址 */
uint32_t addr_v2p(uint32_t vaddr)
{
    uint32_t* pte = pte_ptr(vaddr);
    return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

void block_desc_init(struct mem_block_desc* desc_array)
{
    put_str("block_desc_init start\n");
    uint16_t desc_idx, block_size = 16;
    for(desc_idx = 0;desc_idx<DESC_CNT;desc_idx++)
    {
        desc_array[desc_idx].block_size = block_size;
        desc_array[desc_idx].blocks_per_arena = (PAGE_SIZE - sizeof(struct arena)) / block_size;
        deque_init(&desc_array[desc_idx].free_list);
        block_size *= 2;
    }
    put_str("blokc_desc_init done\n");
}


/* 返回arena中第idx个内存块的地址 */
static struct mem_block* arena2block(struct arena* a, uint32_t idx)
{
    return (struct mem_block*)((uint32_t)a + sizeof(struct arena) + idx * a->desc->block_size);
}

/* 返回内存块b所在的arena地址, 原理类似running_thread() */
static struct arena* block2arena(struct mem_block* b)
{
    return (struct arena*)((uint32_t)b & 0xfffff000);
}

/* 在堆中申请size字节内存 */
void* sys_malloc(uint32_t size)
{
    enum pool_flags PF;
    struct pool* mem_pool;
    uint32_t pool_size;
    struct mem_block_desc* descs;
    struct PCB* cur_thread = running_thread();
    if(cur_thread->pgdir==NULL)
    {
        PF = PF_KERNEL;
        pool_size = kernel_pool.pool_size;
        mem_pool = &kernel_pool;
        descs = k_block_descs;
    }
    else 
    {
        PF = PF_USER;
        pool_size = user_pool.pool_size;
        mem_pool = &user_pool;
        descs = cur_thread->u_block_desc;
    }
    if(!(size > 0 && size < pool_size))
    {
        return NULL;
    }
    struct arena* a ;
    struct mem_block* b;
    lock_acquire(&mem_pool->lck);
    if(size > 1024)
    {
        uint32_t page_cnt = DIV_ROUND_UP(size + sizeof(struct arena), PAGE_SIZE);
        a = malloc_page(PF, page_cnt);
        if(a!=NULL)
        {
            memset(a, 0, PAGE_SIZE * page_cnt);
            a->desc = NULL;
            a->cnt = page_cnt;
            a->large = 1;
            lock_release(&mem_pool->lck);
            return (void*)(a+1);                                 //跨过arena元信息, 返回分配好的内存地址
        }
        else 
        {
            lock_release(&mem_pool->lck);
            return NULL;
        }
    }
    else 
    {
        uint8_t desc_idx;
        for(desc_idx=0;desc_idx<DESC_CNT;desc_idx++)
        {
            if(size<=descs[desc_idx].block_size)
            {
                break;
            }
        }
        if(deque_empty(&descs[desc_idx].free_list))
        {
            a = malloc_page(PF, 1);
            if(a==NULL)
            {
                lock_release(&mem_pool->lck);
                return NULL;
            }
            memset(a, 0, PAGE_SIZE);
            a->desc = &descs[desc_idx];
            a->large = 0;
            a->cnt = descs[desc_idx].blocks_per_arena;
            uint32_t block_idx;
            enum intr_status old_status = intr_disable();
            for(block_idx=0;block_idx<descs[desc_idx].blocks_per_arena;block_idx++)
            {
                b = arena2block(a, block_idx);
                ASSERT(!elem_find(&a->desc->free_list, &b->free_elem));
                deque_append(&a->desc->free_list, &b->free_elem);
            }
            intr_set_status(old_status);
        }
        b = elem_to_entry(struct mem_block, free_elem, deque_pop(&(descs[desc_idx].free_list)));
        memset(b, 0, descs[desc_idx].block_size);
        a = block2arena(b);
        a->cnt--;
        lock_release(&mem_pool->lck);
        return (void*)b;
    }
}


/* 将物理地址pg_phy_addr回收到物理内存池 */
void pfree(uint32_t pg_phy_addr)
{
    struct pool* mem_pool;
    uint32_t bit_idx = 0;
    if(pg_phy_addr >= user_pool.phy_addr_start)
    {
        mem_pool = &user_pool;
        bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PAGE_SIZE;
    }
    else 
    {
        mem_pool = &kernel_pool;
        bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PAGE_SIZE;
    }
    bitmap_setbit(&mem_pool->pool_bitmap, bit_idx, 0);
}

/* 删除页表中虚拟地址vaddr的映射, 即将vaddr对用的pte的P位设为0 */
static void page_table_pte_remove(uint32_t vaddr)
{
    uint32_t* pte = pte_ptr(vaddr);
    *pte &= ~PG_P_1;
    asm volatile("invlpg %0"::"m" (vaddr):"memory"); 
}

/* 在虚拟地址中释放以_vaddr起始的连续pg_cnt个虚拟页地址 */
static void vaddr_remove(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt)
{
    uint32_t bit_idx_start = 0, vaddr = (uint32_t)_vaddr, cnt = 0;
    if(pf==PF_KERNEL)
    {
        bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PAGE_SIZE;
        while(cnt < pg_cnt)
        {
            bitmap_setbit(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
        }
    }
    else
    {
        struct PCB* cur_thread = running_thread();
        bit_idx_start = (vaddr - cur_thread->usrprog_vaddr.vaddr_start) / PAGE_SIZE;
        while(cnt < pg_cnt)
        {
            bitmap_setbit(&cur_thread->usrprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
        }
    }
}

/* 释放以虚拟地址_vaddr起始的pg_cnt个物理页框 */
void mfree_page(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt)
{
    uint32_t pg_phy_addr;
    uint32_t vaddr = (uint32_t)_vaddr, page_cnt = 0;
    ASSERT((pg_cnt >= 1) && ((vaddr % PAGE_SIZE) == 0));
    pg_phy_addr = addr_v2p(vaddr);
    ASSERT(((pg_phy_addr % PAGE_SIZE) == 0) && (pg_phy_addr >= 0x102000)); //0x102000表示低端1MB + 1KB页目录表 + 1KB页表
    if(pg_phy_addr >= user_pool.phy_addr_start)
    {
        vaddr -= PAGE_SIZE;                                            //方便while里面的操作, 第一个先减一下, 后面再加的时候就不需要多做判断
        while(page_cnt < pg_cnt)
        {
            vaddr += PAGE_SIZE;
            pg_phy_addr = addr_v2p(vaddr);
            ASSERT(((pg_phy_addr % PAGE_SIZE)==0) && (pg_phy_addr>=user_pool.phy_addr_start));
            pfree(pg_phy_addr);
            page_table_pte_remove(vaddr);
            page_cnt++;
        }
        vaddr_remove(pf, _vaddr, pg_cnt);
    }
    else 
    {
        vaddr -= PAGE_SIZE;
        while(page_cnt < pg_cnt)
        {
            vaddr += PAGE_SIZE;
            pg_phy_addr = addr_v2p(vaddr);
            ASSERT(((pg_phy_addr % PAGE_SIZE)==0) && (pg_phy_addr>=kernel_pool.phy_addr_start) && (pg_phy_addr<user_pool.phy_addr_start));
            pfree(pg_phy_addr);
            page_table_pte_remove(vaddr);
            page_cnt++;
        }
        vaddr_remove(pf, _vaddr, pg_cnt);
    }
}

void sys_free(void* ptr)
{
    ASSERT(ptr != NULL);
    if(ptr!=NULL)
    {
        enum pool_flags PF;
        struct pool* mem_pool;
        if(running_thread()->pgdir == NULL)
        {
            ASSERT((uint32_t)ptr >= K_HEAP_START);
            PF = PF_KERNEL;
            mem_pool = &kernel_pool;
        }
        else 
        {
            PF = PF_USER;
            mem_pool = &user_pool;
        }
        lock_acquire(&mem_pool->lck);
        struct mem_block* b = ptr;
        struct arena* a = block2arena(b);
        ASSERT((a->large==0) || (a->large==1));
        if(a->desc==NULL && a->large==1)
        {
            mfree_page(PF, a, a->cnt);
        }
        else 
        {
            deque_append(&a->desc->free_list, &b->free_elem);   //将内存块回收到free_list, 再判断arena是否空闲, 若空闲, 释放arena
            if(++a->cnt==a->desc->blocks_per_arena)
            {
                uint32_t block_idx;
                for(block_idx=0;block_idx<a->desc->blocks_per_arena;block_idx++) //先把arena中的内存块从内存块描述符的free_list中去掉, 然后释放arena
                { 
                    struct mem_block* b = arena2block(a, block_idx);
                    ASSERT(elem_find(&a->desc->free_list, &b->free_elem));
                    deque_remove(&b->free_elem);
                }
                mfree_page(PF, a, 1);
            }
        }
        lock_release(&mem_pool->lck);
    }
}

void mem_init()
{
    put_str("mem_init start\n");
    put_int(*(uint32_t*)(0xb03));
    put_char('\n');
    uint32_t mem_bytes_total = (*(uint32_t*)(0xb03));                 //物理地址0xb03存储了本机的物理内存大小
    mem_pool_init(mem_bytes_total);
    block_desc_init(k_block_descs);
    put_str("mem_init done\n");
}
