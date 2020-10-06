; MBR主引导记录
; 2020/07/25 3:17PM
%include "boot.inc"
SECTION MBR vstart=0x7c00
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00
    mov ax,0xb800
    mov gs,ax
    mov ax,0

; --------------------------------------------------------
;INT 0x10 功能号:0x06 描述:上卷窗口(清屏)
; 输入:
; AH 功能号=0x06
; AL=上卷的行数(如果为0,表示全部)
; BH=上卷行属性
; (CL,CH)=窗口左上角(X,Y)位置
; (DL,DH)=窗口右下角(X,Y)位置
; 无返回值

    mov ax,0x600
    mov bx,0x700
    mov cx,0      ;(0,0)
    mov dx,0x184f ;(80,25)

    int 0x10
; --------------------------------------------------------

    

; ---------------------------------------------------------
; INT 0x10 功能号:3 描述:获取光标位置
; 输入:
; AH 功能号=3
; BH=待获取光标的页号
; 输出:
; CH=光标开始行
; CL=光标结束行
; DH=光标所在行号
; DL=光标所在列号

    mov ah,3
    mov bh,0
    
    int 0x10
; ---------------------------------------------------------

    

; ---------------------------------------------------------
; INT 0x10 功能号:13 描述:打印字符串
; 输入:
; AH 功能号=13
; CX=串长度
; AL 设置写字符方式 AL=01:显示字符串,光标跟随移动
; BH=存储要显示的页号
; BL=字符属性 BL=0x02 黑底绿字

    ; mov ax,msg
    ; mov bp,ax     ;es:bp位于字符串首部
    ; mov ax,0x1301
    ; mov cx,msglen-msg
    ; mov bx,0x0002

    ; int 0x10
; ---------------------------------------------------------

; ---------------------------------------------------------
; 直接向显存中写文本
    mov byte [gs:0x00],'H'
    mov byte [gs:0x01],00000010B
    mov byte [gs:0x02],'E'
    mov byte [gs:0x03],00000010B
    mov byte [gs:0x04],'L'
    mov byte [gs:0x05],00000010B
    mov byte [gs:0x06],'L'
    mov byte [gs:0x07],00000010B
    mov byte [gs:0x08],'O'
    mov byte [gs:0x09],00000010B
    mov byte [gs:0x0a],' '
    mov byte [gs:0x0b],00000010B
    mov byte [gs:0x0c],'M'
    mov byte [gs:0x0d],00000010B
    mov byte [gs:0x0e],'B'
    mov byte [gs:0x0f],00000010B
    mov byte [gs:0x10],'R'
    mov byte [gs:0x11],00000010B
    mov byte [gs:0x12],'!'
    mov byte [gs:0x13],00000010B


; ---------------------------------------------------------



; ---------------------------------------------------------
; 调用读取硬盘例程，并且跳转到内存中刚刚读取的硬盘数据的位置
    mov eax,loader_start_sector
    mov bx,loader_base_addr
    mov cx,4
    call rd_disk_m_16

    jmp loader_base_addr
; ---------------------------------------------------------



; ---------------------------------------------------------
; 读写硬盘
; 输入:
; eax=LBA扇区号
; bx=写入数据的内存地址
; cx=输取的扇区的个数
rd_disk_m_16:

    mov esi,eax                   ; 备份eax
    mov di,cx                     ; 备份cx
    
; step_1 设置要读取的扇区数
    mov dx,0x1f2                  ; I/O端口(0x1f2)中存储未读取的扇区数
    mov al,cl 
    out dx,al 
    mov eax,esi                   ; 恢复eax

; step_2 将LBA地址存入0x1f3~0x1f6
    mov dx,0x1f3                  ; 将7~0位存入0x1f3
    out dx,al 

    mov dx,0x1f4                  ; 将15~8位存入0x1f4
    mov cl,8
    shr eax,cl
    out dx,al                     

    mov dx,0x1f5                  ; 将23~16位存入0x1f5
    shr eax,cl
    out dx,al                     

    mov dx,0x1f6                  ; 第24~27位  
    shr eax,cl                     ; 设置7~4位为1110,表示LBA模式
    and al,0x0f 
    or al,0xe0 
    out dx,al   

; step_3 向0x1f7端口写入读命令,0x20
    mov dx,0x1f7
    mov al,0x20
    out dx,al 

; step_4 检测硬盘状态
 .not_ready:
    nop
    in al,dx                     ; 从0x1f7读取表示读取硬盘状态，写入表示读入命令
    and al,0x88
    cmp al,0x08                  ; 第3位为1表示准备好数据传输，第7位为1表示硬盘忙
    jnz .not_ready               ; 没准备好，继续等

; step_5 从0x1f0端口读数据
    mov ax,di                    ; di为要读取的扇区数，一个扇区512字节，每次读一个字，共需要di*512/2次，也就是di*256次
    mov dx,256
    mul dx 
    mov cx,ax 

    mov dx,0x1f0 
.go_on_ready:
    in ax,dx 
    mov [bx],ax 
    add bx,2
    loop .go_on_ready
    
    ret

; ---------------------------------------------------------

    times 510-($-$$) db 0
    db 0x55,0xaa
