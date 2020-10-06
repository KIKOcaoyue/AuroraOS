%include "boot.inc"
SECTION LOADER vstart=loader_base_addr
loader_stack_top equ loader_base_addr

    jmp loader_begin

; ---------------------------------------------
; 构建GDT及其内部的描述符
; 每一个描述符有两个dd，第一个dd是低32位，第二个dd是高32位
    GDT_BASE:
        dd 0x00000000
        dd 0x00000000
    CODE_DESC:
        dd 0x0000ffff
        dd DESC_CODE_HIGH4
    DATA_STACK_DESC:
        dd 0x0000ffff
        dd DESC_DATA_HIGH4
    VIDEO_DESC:
        dd 0x80000007
        dd DESC_VIDEO_HIGH4
    
    
    GDT_SIZE       equ $-GDT_BASE 
    GDT_LIMIT      equ GDT_SIZE-1
    times 60 dq 0                                         ; 预留60个描述符的空位
    SELECTOR_CODE  equ (0x0001 << 3) + TI_GDT + RPL0
    SELECTOR_DATA  equ (0x0002 << 3) + TI_GDT + RPL0 
    SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0
    
    total_mem_bytes dd 0

; 以下是GDT指针，前2个字节是GDT界限，后4个字节是GDT起始地址
; 在汇编中的dd,dw是从低地址向高地址生长，所以先dw一个GDT的界限，然后dd一个GDT的基地址
    gdt_ptr:
        dw GDT_LIMIT 
        dd GDT_BASE

    loadermsg:
        db 'loader in real model'

    ards_buf times 244 db 0
    ards_nr dw 0
; ---------------------------------------------


 loader_begin:


; ---------------------------------------------
; 打印字符串

    mov sp,loader_base_addr 
    mov bp,loadermsg 
    mov cx,20
    mov ax,0x1301
    mov bx,0x001f
    mov dx,0x1000
    int 0x10

; ---------------------------------------------



; ---------------------------------------------
; 获取物理内存大小
; int 15h ax=E801h 获取内存大小, 最大支持4G
    mov ax,0xe801
    int 0x15
; 分为两个step, 先获取低15M内存, 然后获取16MB~4GB内存
; step_1 获取低15M内存
    mov cx,0x400
    mul cx
    shl edx,16
    and eax,0x0000ffff
    or edx,eax 
    add edx,0x100000
    mov esi,edx 

; step_2 获取16M以上内存
    xor eax,eax 
    mov ax,bx 
    mov ecx,0x10000
    mul ecx 
    add esi,eax 

    mov edx,esi 
    jmp .mem_get_ok

 .mem_get_ok:
    mov [total_mem_bytes],edx 
; ---------------------------------------------




; ---------------------------------------------
; 准备进入保护模式
; step_1 打开A20(第20根地址线) 
; step_2 加载GDT
; step_3 将cr0的PE位 置为1

    in al,0x92                                    ; 打开A20 
    or al,0000_0010B 
    out 0x92,al

    lgdt [gdt_ptr]                                ; 加载GDT 

    mov eax,cr0                                   ; 将cr0的PE位 置为1
    or eax,0x00000001
    mov cr0,eax                  

    jmp dword SELECTOR_CODE:protected_mode_start  ; 通过jmp刷新CPU流水线       

; ---------------------------------------------
 [bits 32]
 protected_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax 
    mov esp,loader_stack_top 
    mov ax,SELECTOR_VIDEO
    mov gs,ax 

    mov byte [gs:0x18],'P'
    mov byte [gs:0x19],00000010B
    mov byte [gs:0x1a],'R'
    mov byte [gs:0x1b],00000010B
    mov byte [gs:0x1c],'O'
    mov byte [gs:0x1d],00000010B
    mov byte [gs:0x1e],'T'
    mov byte [gs:0x1f],00000010B




; ---------------------------------------------
; 从硬盘中读取内核
    mov eax,kernel_start_sector                     ; 内核所在的扇区号
    mov ebx,kernel_bin_base_addr                    ; 从硬盘读出后放入的内存地址
    mov ecx,200                                     ; 读200个扇区

    call rd_disk_m_32
; ---------------------------------------------





; ---------------------------------------------
; 开启分页模式
; 共3步
; step_1 创建页目录及页表并初始化内存位图(还有其他一些工作,分别是取回GDT, 修改显存基地址, 修改栈指针)
    call setup_page

    sgdt [gdt_ptr]                                 ; 将GDT中显存段描述符的基地址+0xc0000000, 因为虚拟内存的3GB~4GB空间为内核空间, 对应的地址为0xc0000000~0xffffffff

    mov ebx,[gdt_ptr + 2]
    or dword [ebx + 0x18 + 0x4],0xc0000000         ; 显存段在0x18, 描述符中的基地址的最高位存放在高4字节的最高一个字节, 故需要加上0x4, 将低四位排出去

    add dword [gdt_ptr + 2],0xc0000000             ; 将GDT的基地址加上0xc0000000, 使其进入内核空间
    add esp,0xc0000000                             ; 栈指针也要指向内核地址(0xc0000000) 

; step_2 将页目录地址赋给cr3
    mov eax,page_dir_table_pos
    mov cr3,eax 

; step_3 打开cr0的pg位
    mov eax,cr0
    or eax,0x80000000
    mov cr0,eax 
; ---------------------------------------------



; ---------------------------------------------
; 开启分页后，重新加载GDT
    lgdt [gdt_ptr]
; ---------------------------------------------

; ---------------------------------------------
; 重新访问显存进行测试
    ; mov byte [gs:0x20],'V'
    ; mov byte [gs:0x21],00000010B
    ; mov byte [gs:0x22],'T'
    ; mov byte [gs:0x23],00000010B
    ; mov byte [gs:0x24],'M'
    ; mov byte [gs:0x25],00000010B
; ---------------------------------------------


; ---------------------------------------------
; 刷新流水线
    jmp SELECTOR_CODE:enter_kernel

enter_kernel:
    call kernel_init
    mov esp,0xc009f000
    jmp kernel_entry_point                          ; 进入内核
; ---------------------------------------------



; ---------------------------------------------
; 创建页目录及页表(内核)
setup_page:

    mov ecx,4096                                    ; 将页目录表各项清零
    mov esi,0
 .clear_page_dir:
    mov byte [page_dir_table_pos + esi],0
    inc esi 
    loop .clear_page_dir


 .create_pde:                                       ; 创建页目录项(PDE)
    mov eax,page_dir_table_pos
    add eax,0x1000
    mov ebx,eax                                    
    or eax,PG_US_U | PG_RW_W | PG_P                 ; 设置页目录项的属性
    mov [page_dir_table_pos + 0x0],eax              ; 在页目录表的第1个项中写入第一个页表的位置(0x101000)及属性
    mov [page_dir_table_pos + 0xc00],eax            ; 在页目录表的第768个项中写入第一个页表的位置(0x101000)及属性
    sub eax,0x1000
    mov [page_dir_table_pos + 4092],eax             ; 在页目录表的最后一个项中写入自己的地址(方便日后操作)


    mov ecx,256                                     ; 创建页表项(PTE), 内核在低端1M内, 所以1M / 4KB(页的大小) = 256(个页表项), 现在共需要填充256个页表项
    mov esi,0
    mov edx,PG_US_U | PG_RW_W | PG_P 
 .create_pte:
    mov [ebx+esi*4],edx
    add edx,4096                                    ; 0x1000 = 4096 = (4KB), 第一个页表的页表项的虚拟地址与物理地址一致(相等),(第一个是0,第二个是0+4096,第三个是0+4096+4096...)
    inc esi 
    loop .create_pte 


    mov eax,page_dir_table_pos                      ; 创建内核其他页表的PDE
    add eax,0x2000                                  ; 第二个页表
    or eax,PG_US_U | PG_RW_W | PG_P
    mov ebx,page_dir_table_pos
    mov ecx,254                                     ; 一共256个, 第1个和第768都填充好了, 现在要填充769~1022号(一共是0~1023)
    mov esi,769
 .create_kernel_pde:
    mov [ebx+esi*4],eax
    add eax,0x1000
    inc esi 
    loop .create_kernel_pde

; ---------------------------------------------

    ret 

; ---------------------------------------------
; 读取硬盘
rd_disk_m_32:

    mov esi,eax                   ; 备份eax
    mov di,cx                   ; 备份ecx
    
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
    mov [ebx],ax 
    add ebx,2
    loop .go_on_ready
    
    ret


; ---------------------------------------------
; 根据elf规范解析内核文件, 即将内核文件中的段复制到内存的相应位置
kernel_init:
    xor eax,eax
    xor ebx,ebx                                   ; ebx记录程序头表地址
    xor ecx,ecx                                   ; cx记录程序头表中program header数量
    xor edx,edx                                   ; dx记录program header尺寸, 即e_phentsize

    mov dx,[kernel_bin_base_addr + 42]            ; 偏移文件42字节处的属性是e_phentsize
    mov ebx,[kernel_bin_base_addr + 28]           ; 偏移文件28字节处的属性是e_phoff, 表示第1个program header在文件中的偏移量

    add ebx,kernel_bin_base_addr
    mov cx,[kernel_bin_base_addr + 44]            ; 偏移文件44字节处属性是e_phnum, 表示program header的数量

 .each_segment:
    cmp byte [ebx + 0],PT_NULL                    ; 若p_type等于PT_NULL, 说明此program header未被使用
    je .PTNULL 

; 为函数mem_cpy压入参数, 参数是从右往左依次进入, 函数原型为memcpy(dst,src,size)
    push dword [ebx + 16]                         ; program header偏移16个字节是p_filesz, 即函数中size
    mov eax,[ebx + 4]                             ; program header偏移4个字节是p_offset
    add eax,kernel_bin_base_addr                  ; 加上kernel.bin被加载到的物理地址, eax为该段的源物理地址
    push eax                                      ; 压入第二个参数,src
    push dword [ebx + 8]                          ; 压入第一个参数dst

    call mem_cpy

    add esp,12                                    ; 清理栈中压入的三个参数

 .PTNULL:
    add ebx,edx                                   ; edx为program header的大小, 至此ebx指向下一个program header

    loop .each_segment

    ret 
; ---------------------------------------------




; ----------------------------------------------
; 逐字节拷贝
; mem_cpy(dst,src,size)
mem_cpy:
    cld                                           ; 方向指令, 控制movsb的拷贝方向, cld意为movsb一次就自动加1, sld意为movsb一次就自动减1
    push ebp 
    mov ebp,esp 
    push ecx                                      ; rep使用了ecx, 先备份

    mov edi,[ebp + 8]                             ; dst
    mov esi,[ebp + 12]                            ; src
    mov ecx,[ebp + 16]                            ; size
    rep movsb

    pop ecx                                       ; 恢复环境
    pop ebp 

    ret
; ----------------------------------------------