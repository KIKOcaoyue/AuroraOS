#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"
#include "print.h"
#define PIC_M_CTRL   0x20
#define PIC_M_DATA   0x21
#define PIC_S_CTRL   0xa0
#define PIC_S_DATA   0xa1

#define IDT_DESC_CNT 0x81                    //目前总共支持的中断数, syscall为中断0x80, 故需要0~0x80共0x81个中断

#define EFLAGS_IF    0x00000200
#define GET_FLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0" : "=g" (EFLAG_VAR))

/* 中断门描述符结构体 */
struct gate_desc
{
    uint16_t func_offset_low_word;
    uint16_t selector;
    uint8_t  dcount;
    uint8_t  attribute;
    uint16_t func_offset_high_word;
};

static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];

extern intr_handler intr_entry_table[IDT_DESC_CNT];
char* intr_name[IDT_DESC_CNT];
intr_handler idt_table[IDT_DESC_CNT];

extern uint32_t syscall_handler(void);     //syscall中断入口例程, kernel.asm中实现

static void pic_init()
{

    outb(PIC_M_CTRL, 0x11);
    outb(PIC_M_DATA, 0x20);
    outb(PIC_M_DATA, 0x04);
    outb(PIC_M_DATA, 0x01);

    outb(PIC_S_CTRL, 0x11);
    outb(PIC_S_DATA, 0x28);
    outb(PIC_S_DATA, 0x02);
    outb(PIC_S_DATA, 0x01);

    outb(PIC_M_DATA, 0xf8);
    outb(PIC_S_DATA, 0xbf);

    put_str("pic_init done\n");
}

/* 创建中断门描述符 */
static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr, intr_handler function)
{
    p_gdesc->func_offset_low_word  = (uint32_t) function & 0x0000ffff;
    p_gdesc->selector              = SELECTOR_K_CODE;
    p_gdesc->dcount                = 0;
    p_gdesc->attribute             = attr;
    p_gdesc->func_offset_high_word = ((uint32_t) function & 0xffff0000) >> 16;
}

/* 初始化中断描述符表 */
static void idt_desc_init()
{
    int i, lst = IDT_DESC_CNT - 1;
    for(i=0;i<IDT_DESC_CNT;i++)
    {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
    make_idt_desc(&idt[lst], IDT_DESC_ATTR_DPL3, syscall_handler);
    put_str("idt_desc_init done\n");
}

/* 通用的中断处理函数 */
static void general_intr_handler(uint8_t vec_nr)
{
    if(vec_nr==0x27 || vec_nr==0x2f)                 //IRQ7和IRQ15会产生伪中断, 无需处理. 0x2f是从片上的最后一个IRQ引脚, 保留项
    {
        return;
    }
    set_cursor(0);
    int cursor_pos = 0;
    while(cursor_pos < 320)
    {
        put_char(' ');
        cursor_pos++;
    }
    set_cursor(0);
    put_str("\n-------------------------------- excetion message begin ----------------------------------\n");
    set_cursor(88);
    put_str(intr_name[vec_nr]);
    if(vec_nr==14)
    {
        int page_fault_vaddr = 0;
        asm ("movl %%cr2, %0" : "=r" (page_fault_vaddr));
        put_str("\npage fault addr is: "); put_int(page_fault_vaddr);
    }
    put_str("\n-------------------------------- excetion message end ----------------------------------\n");
    while(1);
}

/* 通用中断处理函数的注册和异常名称注册 */
static void exception_init(void)
{
    int i;
    for(i=0;i<IDT_DESC_CNT;i++)
    {
        idt_table[i] = general_intr_handler;
        intr_name[i] = "unknown";
    }
    intr_name[0]  = "#DE Divide Error";
    intr_name[1]  = "#DB Debug Exception";
    intr_name[2]  = "NMI Interrupt";
    intr_name[3]  = "#BP Breakpoint Exception";
    intr_name[4]  = "#OF Overflow Exception";
    intr_name[5]  = "#BR BOUND Range Exceeded Exception";
    intr_name[6]  = "#UD Invalid Opcode Exception";
    intr_name[7]  = "#NM Device Not Available Exception";
    intr_name[8]  = "#DF Double Fault Exception";
    intr_name[9]  = "#Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP General Protection Exception";
    intr_name[14] = "#PF Page Fault Exception";
    //intr_name[15] = "第15项是Intel保留项, 未使用";
    intr_name[16] = "#MF x87 FPU Floating-Point Exception";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine Check Exception";
    intr_name[19] = "#XF SIMD Floating Point Exception";
}


void register_handler(uint8_t vector_no, intr_handler function)
{
    idt_table[vector_no] = function;
}

/* 将中断状态设置为status */
enum intr_status intr_set_status(enum intr_status status)
{
    return status & INTR_ON ? intr_enable() : intr_disable();
}


/* 获取当前中断状态 */
enum intr_status intr_get_status()
{
    uint32_t eflags = 0;
    GET_FLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}


/* 开中断并返回开中断前的状态 */
enum intr_status intr_enable()
{
    enum intr_status old_status;
    if(INTR_ON==intr_get_status())
    {
        old_status = INTR_ON;
        return old_status;
    }
    else
    {
        old_status = INTR_OFF;
        asm volatile("sti");
        return old_status;
    }
}


/* 关中断并返回关中断前的状态 */
enum intr_status intr_disable()
{
    enum intr_status old_status;
    if(INTR_ON==intr_get_status())
    {
        old_status = INTR_ON;
        asm volatile("cli" : : : "memory");
        return old_status;
    }
    else
    {
        old_status = INTR_OFF;
        return old_status;
    }
}




/* 完成有关中断的所有初始化工作 */
void idt_init()
{
    put_str("idt_init start\n");
    idt_desc_init();
    exception_init();
    pic_init();

    /* 加载IDT */
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t) (uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
    put_str("idt init done\n");
}

