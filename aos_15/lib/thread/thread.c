/*
    Author: caoyue
    Date: 2020/08/08 10:29 AM
*/
#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "interrupt.h"
#include "debug.h"
#include "process.h"

struct PCB* idle_thread;

static void idle(void* arg)
{
    while(1)
    {
        thread_block(TASK_BLOCKED);
        asm volatile("sti; hlt" : : : "memory");
    }
}

static pid_t allocate_pid()
{
    static pid_t next_pid = 0;
    lock_acquire(&pid_lck);
    next_pid++;
    lock_release(&pid_lck);
    return next_pid;
}

/*
    返回线程的PCB地址
    由于各个线程所用的0级栈都在自己的PCB中, 故取当前栈指针esp的高20位, 即指向了当前的PCB
    因为地址共32位, 其中高20位是所在页的基地址, 后12位是页内偏移
    (本人自己的理解, 可能是不对的, 姑且算作笔记, 若有大神看出错误, 请电邮指点)
    电邮不出意外是在github主页上
*/
struct PCB* running_thread()                                 
{                                                            
    uint32_t esp;
    asm ("mov %%esp, %0" : "=g" (esp));
    return (struct PCB*) (esp & 0xfffff000);
}

/* 由kernel_thread去执行function(func_arg) */
static void kernel_thread(thread_func* function, void* func_arg)
{
    intr_enable();
    function(func_arg);
}

/* 初始化线程栈 */
void thread_create(struct PCB* pthread, thread_func function, void* func_arg)
{
    pthread->self_kstack -= sizeof(struct intr_stack);                         //预留出中断使用栈的空间, 可见thread.h中定义的结构
    pthread->self_kstack -= sizeof(struct thread_stack);                       //预留出线程栈的空间
    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;
}

/* 初始化线程基本信息 */
void init_thread(struct PCB* pthread, char* name, int prio)
{
    memset(pthread, 0, sizeof(*pthread));
    pthread->pid = allocate_pid();
    strcpy(pthread->name, name);
    if(pthread==main_thread)
    {
        pthread->status = TASK_RUNNING;
    }
    else
    {
         pthread->status = TASK_READY;
    }
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PAGE_SIZE);         //self_kstack是线程自己在内核态下使用的栈顶地址
    pthread->priority = prio;
    pthread->ticks = prio;
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;
    memset(pthread->u_block_desc, 0, DESC_CNT * sizeof(struct mem_block_desc));
    pthread->fd_table[0] = 0;
    pthread->fd_table[1] = 1;
    pthread->fd_table[2] = 2;
    uint8_t fd_idx = 3;
    while(fd_idx < MAX_FILES_OPEN_PER_PROC)
    {
        pthread->fd_table[fd_idx] = -1;
        fd_idx++;
    }
    pthread->cwd_inode_nr = 0;
    pthread->stack_magic = 0x20200620;                                         //自定义的魔数
}

/* 创建一个优先级为prio的线程, 线程名为name, 线程执行的函数为function(func_arg) */
struct PCB* thread_start(char* name, int prio, thread_func function, void* func_arg)
{
    struct PCB* thread = get_kernel_pages(1); //分配了一个页, 得到了页的地址
    init_thread(thread, name, prio);
    thread_create(thread, function, func_arg);
    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    deque_append(&thread_ready_list, &thread->general_tag);
    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    deque_append(&thread_all_list, &thread->all_list_tag);
    return thread;
}

static void make_main_thread()
{
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);
    ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    deque_append(&thread_all_list, &main_thread->all_list_tag);
}

void schedule()
{
    ASSERT(intr_get_status()==INTR_OFF);
    struct PCB* cur = running_thread();
    if(cur->status==TASK_RUNNING)
    {
        ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        deque_append(&thread_ready_list, &cur->general_tag);
        cur->ticks = cur->priority;
        cur->status = TASK_READY;
    }
    else 
    {
        
    }
    if(deque_empty(&thread_ready_list))
    {
        thread_unblock(idle_thread);
    }
    ASSERT(!deque_empty(&thread_ready_list));
    thread_tag = NULL;
    thread_tag = deque_pop(&thread_ready_list);
    struct PCB* next = elem_to_entry(struct PCB, general_tag, thread_tag);
    next->status = TASK_RUNNING;
    process_activate(next);
    switch_to(cur,next);
}

void thread_block(enum task_status stat)
{
    ASSERT(((stat == TASK_BLOCKED) || (stat == TASK_WAITING) || (stat == TASK_HANGING)));
    enum intr_status old_status = intr_disable();
    struct PCB* cur_thread = running_thread();
    cur_thread->status = stat;
    schedule();
    intr_get_status(old_status);
}

void thread_unblock(struct PCB* pthread)
{
    enum intr_status old_status = intr_disable();
    ASSERT(((pthread->status == TASK_BLOCKED) || (pthread->status == TASK_WAITING) || (pthread->status == TASK_HANGING)));
    if(pthread->status != TASK_READY)
    {
        ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag));
        if(elem_find(&thread_ready_list, &pthread->general_tag))
        {
            PANIC("thread_unblock: blocked thread in ready_list\n");
        }
        deque_push(&thread_ready_list, &pthread->general_tag);
        pthread->status = TASK_READY;
    }
    intr_set_status(old_status);
}

/* 主动让出CPU, 但是不阻塞 */
void thread_yield()
{
    struct PCB* cur = running_thread();
    enum intr_status old_status = intr_disable();
    ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
    deque_append(&thread_ready_list, &cur->general_tag);
    cur->status = TASK_READY;
    schedule();
    intr_set_status(old_status);
}

void thread_init()
{
    put_str("thread_init start\n");
    deque_init(&thread_ready_list);
    deque_init(&thread_all_list);
    lock_init(&pid_lck);
    make_main_thread();
    idle_thread = thread_start("idle", 10, idle, NULL);
    put_str("thread_init done\n");
}


