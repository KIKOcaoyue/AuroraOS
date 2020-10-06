#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "stdint.h"
#include "deque.h"
#include "process.h"
#include "sync.h"
#include "memory.h"

#define PAGE_SIZE               4096
#define MAX_FILES_OPEN_PER_PROC 8

struct PCB* main_thread;
struct DEQUE thread_ready_list;   //就绪队列
struct DEQUE thread_all_list;     //所有线程队列
static struct DEQUE_ELEM* thread_tag;

extern void switch_to(struct PCB* cur, struct PCB* next);

typedef void thread_func(void*);
typedef uint32_t pid_t;

enum task_status
{
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

/*
    中断栈 intr_stack
    用于中断发生时保护程序的上下文环境:
    进程或线程被外部中断或软中断打断时, 会按照此结构压入上下文
    寄存器, intr_exit中的出栈操作是此结构的逆操作
    此栈在线程自己的内核栈中位置固定, 所在页的最顶端
*/
struct intr_stack
{
    uint32_t vec_no;                     //kernel.asm 宏VECTOR中push %1压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    /* 以下由CPU从低特权级进入高特权级时压入 */
    uint32_t err_code;
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void*    esp;
    uint32_t ss;
};

/*
    线程栈 thread_stack
    线程自己的栈, 用于存储线程中待执行的函数
    此结构在线程自己的内核栈中位置不固定
    仅用在switch_to时保存线程环境
    实际位置取决于实际运行情况
*/
struct thread_stack
{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /* 线程第一次执行时, eip指向待调用的函数kernel_thread, 其他时候, eip指向switch_to的返回地址 */
    void (*eip) (thread_func* func, void* func_arg);

    /* 以下仅供第一次被调度上CPU时使用 */
    void (*unused_retaddr);
    thread_func* function;
    void* func_arg;
};

struct PCB
{
    uint32_t*             self_kstack;
    pid_t                 pid;
    enum task_status      status;
    char                  name[16];
    uint8_t               priority;
    uint8_t               ticks;
    uint32_t              elapsed_ticks;
    int32_t               fd_table[MAX_FILES_OPEN_PER_PROC];
    struct DEQUE_ELEM     general_tag;
    struct DEQUE_ELEM     all_list_tag;
    uint32_t*             pgdir;
    struct virtual_addr   usrprog_vaddr;
    struct mem_block_desc u_block_desc[DESC_CNT];
    uint32_t              cwd_inode_nr;                  //进程所在工作目录的inode编号
    uint32_t              stack_magic;
};

struct lock pid_lck;

static pid_t allocate_pid();

struct PCB* running_thread();

/* 由kernel_thread去执行function(func_arg) */
static void kernel_thread(thread_func* function, void* func_arg);

/* 初始化线程栈 */
void thread_create(struct PCB* pthread, thread_func function, void* func_arg);

/* 初始化线程基本信息 */
void init_thread(struct PCB* pthread, char* name, int prio);

/* 创建一个优先级为prio的线程, 线程名为name, 线程执行的函数为function(func_arg) */
struct PCB* thread_start(char* name, int prio, thread_func function, void* func_arg);

static void make_main_thread();

void schedule();

void thread_block(enum task_status stat);

void thread_unblock(struct PCB* pthread);

void thread_init();




#endif 