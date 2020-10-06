[bits 32]
%define ERROR_CODE nop                   ; 有的中断会压入错误码, 有的不会, 那么在不会压入错误码的中断号中放入一个0, 这样, 就能保证栈ret时的esp一定指向eip
%define ZERO push 0

extern put_str
extern put_int
extern idt_table

section .data

global intr_entry_table
intr_entry_table:

%macro VECTOR 2
section .text 
intr%1entry:

    %2
    
    push ds
    push es
    push fs 
    push gs 
    pushad 

    mov al,0x20                          ; 中断结束命令EOI
    out 0xa0,al                          ; 向从片发送
    out 0x20,al                          ; 向主片发送

    push %1 
    call [idt_table + %1 * 4]
    jmp intr_exit

section .data
    dd intr%1entry                       ; 存储各个中断入口程序的地址, 形成intr_entry_table数组
%endmacro 

section .text 
global intr_exit
intr_exit:
    add esp,4
    popad 
    pop gs 
    pop fs 
    pop es 
    pop ds 

    add esp,4

    iretd


; -------------------------------------------
VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE 
VECTOR 0x1f,ZERO
VECTOR 0x20,ZERO
VECTOR 0x21,ZERO
VECTOR 0x22,ZERO
VECTOR 0x23,ZERO
VECTOR 0x24,ZERO
VECTOR 0x25,ZERO
VECTOR 0x26,ZERO
VECTOR 0x27,ZERO
VECTOR 0x28,ZERO
VECTOR 0x29,ZERO
VECTOR 0x2a,ZERO
VECTOR 0x2b,ZERO
VECTOR 0x2c,ZERO
VECTOR 0x2d,ZERO
VECTOR 0x2e,ZERO
VECTOR 0x2f,ZERO


; 0x80号中断
[bits 32]
extern syscall_table
section .text
global syscall_handler
syscall_handler:
; step1: 保存上下文
    push 0                            ; 压入0, 使栈中格式统一

    push ds
    push es 
    push fs
    push gs 
    pushad                           

    push 0x80                         ; 压入0x80, 使栈中格式统一  
; step2: 为syscall子功能传入参数
    push edx                          ; 第3个参数
    push ecx                          ; 第2个参数
    push ebx                          ; 第1个参数
; step3: 调用子功能处理函数
    call [syscall_table + eax * 4]
    add esp,12                        ; 跨过上面三个参数
; step4: 将调用后的返回值存入当前内核栈中eax的位置
    mov [esp + 8 * 4],eax             ; (1+7)*4, 其中1表示push 0x80中的4字节, push ad会压入8个寄存器, eax是最先压入, 故使用7来表示其偏移
    jmp intr_exit 