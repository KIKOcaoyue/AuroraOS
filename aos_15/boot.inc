; -------------------------------------------
; loader and kernel
loader_base_addr      equ 0x900
loader_start_sector   equ 0x2
page_dir_table_pos    equ 0x100000
kernel_start_sector   equ 0x9
kernel_bin_base_addr  equ 0x70000
kernel_entry_point    equ 0xc0001500
; -------------------------------------------

; -------------------------------------------
; GDT描述符属性
; DESC_字段名_字段信息
    DESC_G_4K         equ 1_000_0000_0000_0000_0000_0000B
    DESC_D_32         equ 1_00_0000_0000_0000_0000_0000B
    DESC_L            equ 0_0_0000_0000_0000_0000_0000B
    DESC_AVL          equ 0_0000_0000_0000_0000_0000B 
    DESC_LIMIT_CODE2  equ 1111_0000_0000_0000_0000B
    DESC_LIMIT_DATA2  equ DESC_LIMIT_CODE2 
    DESC_LIMIT_VIDEO2 equ 0000_000_0000_0000_0000B
    DESC_P            equ 1_000_0000_0000_0000B 
    DESC_DPL_0        equ 00_0_0000_0000_0000B 
    DESC_DPL_1        equ 01_0_0000_0000_0000B
    DESC_DPL_2        equ 10_0_0000_0000_0000B 
    DESC_DPL_3        equ 11_0_0000_0000_0000B 
    DESC_S_CODE       equ 1_0000_0000_0000B 
    DESC_S_DATA       equ DESC_S_CODE 
    DESC_S_sys        equ 0_0000_0000_0000B 
    DESC_TYPE_CODE    equ 1000_0000_0000B 
    DESC_TYPE_DATA    equ 0010_0000_0000B 
    DESC_CODE_HIGH4   equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
    DESC_DATA_HIGH4   equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
    DESC_VIDEO_HIGH4  equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b

; -------------------------------------------

; -------------------------------------------
; 选择子属性
    RPL0              equ 00B 
    RPL1              equ 01B 
    RPL2              equ 10B 
    RPL3              equ 11B 
    TI_GDT            equ 000B 
    TI_LDT            equ 100B 
; -------------------------------------------



; -------------------------------------------
; 页表相关属性
; PG_US_S例如: PTE和PDE的US属性值为S
    PG_P              equ 1B
    PG_RW_R           equ 00B 
    PG_RW_W           equ 10B 
    PG_US_S           equ 000B 
    PG_US_U           equ 100B  
; -------------------------------------------



; -------------------------------------------
; 内核相关属性
    PT_NULL           equ 0x0
; -------------------------------------------