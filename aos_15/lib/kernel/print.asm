; print系统调用
; 2020/07/29 10:33AM
; Author: caoyue
%include "print.inc"


[bits 32]
section .data
put_int_buffer dq 0

section .text

; ----------------------------------------------
; put_int
; 实现整数打印
global put_int
put_int:
    pushad 
    
    mov ebp,esp
    mov eax,[ebp + 4 * 9]                       ; call的返回地址占4字节+pushad的8个4字节
    mov edx,eax 
    mov edi,7                                   ; 指定在put_int_buffer中的初始偏移量
    mov ecx,8                                   ; 32位数字中, 十六进制数字的位数是8个
    mov ebx,put_int_buffer

 .16based_4bits:
    and edx,0x0000000f
    cmp edx,9
    jg .is_A_to_F
    add edx,'0'
    jmp .store 

 .is_A_to_F:
    sub edx,10
    add edx,'A'
 
 .store:
    mov [ebx + edi],dl
    dec edi
    shr eax,4
    mov edx,eax
    loop .16based_4bits

 .ready_to_print:
    inc edi
 .skip_prefix_0:
    cmp edi,8
    je .full0

 .go_on_skip:
    mov cl,[put_int_buffer + edi]
    inc edi 
    cmp cl,'0'
    je .skip_prefix_0
    dec edi 
    jmp .put_each_num

 .full0:
    mov cl,'0'
 .put_each_num:
    push ecx 
    call put_char 
    add esp,4
    inc edi 
    mov cl,[put_int_buffer + edi]
    cmp edi,8
    jl .put_each_num
     
    popad
    ret 

; ----------------------------------------------



; ----------------------------------------------
; put_str
; 打印栈中的字符串
global put_str
put_str:
    push ebx
    push ecx 
    xor ecx,ecx 
    mov ebx,[esp + 12]                          ; 从栈中得到待打印字符串的地址

 .goon:
    mov cl,[ebx]
    cmp cl,0
    jz .str_over
    push ecx 
    call put_char 
    add esp,4
    inc ebx 
    jmp .goon 

 .str_over:
    pop ecx 
    pop ebx 

    ret 

; ----------------------------------------------



; ----------------------------------------------
; put_char
; 把栈中的1个字符写入光标所在处
global put_char
put_char:
    pushad                                      ; 备份32位寄存器环境, 入栈顺序为eax->ecx->edx->ebx->esp->ebp->esi->edi
    
    mov ax,SELECTOR_VIDEO
    mov gs:ax                                   ; 保证gs指向显存地址, 每次都赋值确保正确

; step_1 先获取当前光标位置
; 获取光标位置的高8位
    mov dx,0x03d4                               ; 索引寄存器                               
    mov al,0x0e                                 ; 用于提供光标位置的高8位
    out dx,al
    mov dx,0x03d5                               ; 通过读写数据端口0x03d5来获得或设置光标位置
    in al,dx
    mov ah,al                      
; 获取光标位置的低8位
    mov dx,0x03d4
    mov al,0x0f
    out dx,al 
    mov dx,0x03d5
    in al,dx 

    mov bx,ax                                   ; 将光标位置存入bx

; step_2 主控制分支

    mov ecx,[esp + 36]                          ; 获取在栈中待打印的字符数: pushad压入的4*8=32(字节), 加上主调函数4字节的返回地址, 故esp + 36

    cmp cl,0x0d                                 ; ASCII(0x0d)对应CR
    jz .is_carriage_return
    cmp cl,0x0a                                 ; (0x0a)对应LF
    jz .is_line_feed
    cmp cl,0x08                                 ; (0x08)对应BS(backspace)
    jz .is_backspace

    jmp .put_other 

; step_3 主控制分支下的各分支
    
 .is_backspace:
    dec bx 
    shl bx,1                                    
    mov byte [gs:bx],0x20
    inc bx
    mov byte [gs:bx],0x07
    shr bx,1
    jmp set_cursor

 .put_other:
    shl bx,1
    mov [gs:bx],cl
    inc bx
    mov byte [gs:bx],0x07
    shr bx,1
    inc bx
    cmp bx,2000
    jl set_cursor

 .is_line_feed:

 .is_carriage_return:
    xor dx,dx
    mov ax,bx
    mov si,80
    div si
    sub bx,dx 
 
 .is_carriage_return_end:
    add bx,80
    cmp bx,2000
 
 .is_line_feed_end:
    jl set_cursor

 .roll_screen:
    cld
    mov ecx,960                                 ; 一共1920个字符, 共1920*2 = 3840字节, 一次搬四个字节, 需要3840/4=960次

    mov esi,0xc00b80a0                          ; 第1行行首
    mov edi,0xc00b8000                          ; 第0行行首
    rep movsd

    mov ebx,3840                                ; 将最后一行填成空白, 最后一行行首3840, 需要80个字符
    mov ecx,80

 .cls:
    mov word [gs:ebx],0x720                     ; 0x720是黑底白字的空格
    add ebx,2
    loop .cls
    mov bx,1920                                 ; 将光标重置为最后一行首地址

global set_cursor
 set_cursor:                                   ; 将光标设置为bx值
    ; 先设置高位
    mov dx,0x03d4
    mov al,0x0e
    out dx,al
    mov dx,0x03d5
    mov al,bh
    out dx,al 
    ; 再设置低位
    mov dx,0x03d4
    mov al,0x0f
    out dx,al
    mov dx,0x03d5
    mov al,bl
    out dx,al 

 .put_char_done:
    popad 

    ret 
     
; ---------------------------------------------- 
