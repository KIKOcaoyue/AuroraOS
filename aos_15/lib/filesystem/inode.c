#include "inode.h"
#include "interrupt.h"
#include "debug.h"
#include "thread.h"
#include "ide.h"
#include "file.h"
#include "fs.h"

struct inode_position
{
    bool     two_sec;                  //inode是否跨扇区
    uint32_t sec_lba;                  //inode所在的扇区号
    uint32_t off_size;                 //inode在扇区内的字节偏移量
};


/* 获取inode所在的扇区和扇区内的偏移量 */
static void inode_locate(struct PARTITION* part, uint32_t inode_no, struct inode_position* inode_pos)
{
    ASSERT(inode_no<4096);
    uint32_t inode_table_lba = part->sb->inode_table_lba;
    uint32_t inode_size = sizeof(struct inode);
    uint32_t off_size = inode_no * inode_size;
    uint32_t off_sec = off_size / 512;
    uint32_t off_size_in_sec = off_size % 512;
    uint32_t left_in_sec = 512 - off_size_in_sec;
    if(left_in_sec < inode_size)
    {
        inode_pos->two_sec = true;
    }
    else 
    {
        inode_pos->two_sec = false;
    }
    inode_pos->sec_lba = inode_table_lba + off_sec;
    inode_pos->off_size = off_size_in_sec;
}

/* 将inode写入磁盘分区part */
void inode_sync(struct PARTITION* part, struct inode* inode, void* io_buf)
{
    uint8_t inode_no = inode->i_no;
    struct inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);
    ASSERT(inode_pos.sec_lba<=(part->start_lba+part->sec_cnt));
    struct inode pure_inode;
    memcpy(&pure_inode, inode, sizeof(struct inode));
    pure_inode.i_open_cnts = 0;
    pure_inode.write_deny = false;
    pure_inode.inode_tag.prev = pure_inode.inode_tag.next = NULL;
    char* inode_buf = (char*)io_buf;
    if(inode_pos.two_sec)
    {
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
        memcpy((inode_buf+inode_pos.off_size), &pure_inode, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    }
    else 
    {
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
        memcpy((inode_buf+inode_pos.off_size), &pure_inode, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
}

/* 根据I结点号返回对应的I结点 */
struct inode* inode_open(struct PARTITION* part, uint32_t inode_no)
{
    struct DEQUE_ELEM* elem = part->open_inodes.head.next;
    struct inode* inode_found;
    while(elem != &part->open_inodes.tail)
    {
        inode_found = elem_to_entry(struct inode, inode_tag, elem);
        if(inode_found->i_no==inode_no)
        {
            inode_found->i_open_cnts++;
            return inode_found;
        }
        elem = elem->next;
    }
    struct inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);
    struct PCB* cur = running_thread();
    uint32_t* cur_pagedir_bak = cur->pgdir;
    cur->pgdir = NULL;
    inode_found = (struct inode*)sys_malloc(sizeof(struct inode));
    cur->pgdir = cur_pagedir_bak;
    char* inode_buf;
    if(inode_pos.two_sec)
    {
        inode_buf = (char*)sys_malloc(1024);
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    }
    else 
    {
        inode_buf = (char*)sys_malloc(512);
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
    memcpy(inode_found, inode_buf+inode_pos.off_size, sizeof(struct inode));
    deque_push(&part->open_inodes, &inode_found->inode_tag);
    inode_found->i_open_cnts = 1;
    sys_free(inode_buf);
    return inode_found;
}

/* 关闭I结点或减少I结点打开的次数 */
void inode_close(struct inode* inode)
{
    enum intr_status old_status = intr_disable();
    if(--inode->i_open_cnts == 0)
    {
        deque_remove(&inode->inode_tag);
        struct PCB* cur = running_thread();
        uint32_t* cur_pagedir_bak = cur->pgdir;
        cur->pgdir = NULL;
        sys_free(inode);
        cur->pgdir = cur_pagedir_bak;
    }
    intr_set_status(old_status);
}

/* 初始化new inode */
void inode_init(uint32_t inode_no, struct inode* new_inode)
{
    new_inode->i_no = inode_no;
    new_inode->i_size = 0;
    new_inode->i_open_cnts = 0;
    new_inode->write_deny = false;

    uint8_t sec_idx = 0;
    while(sec_idx<13)
    {
        new_inode->i_sectors[sec_idx] = 0;
        sec_idx++;
    }
}

/* 将硬盘分区part上的inode清空 */
void inode_delete(struct PARTITION* part, uint32_t inode_no, void* io_buf)
{
    ASSERT(inode_no < 4096);
    struct inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);
    ASSERT(inode_pos.sec_lba<=(part->start_lba+part->sec_cnt));

    char* inode_buf = (char*)io_buf;
    if(inode_pos.two_sec)
    {
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
        memset((inode_buf+inode_pos.off_size), 0, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    }
    else 
    {
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
        memset((inode_buf+inode_pos.off_size), 0, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
}

/* 回收inode的数据块和inode本身 */
void inode_release(struct PARTITION* part, uint32_t inode_no)
{
    struct inode* inode_to_del = inode_open(part, inode_no);
    ASSERT(inode_to_del->i_no == inode_no);
    //1. 回收inode占用的所有块
    uint8_t block_idx = 0, block_cnt = 12;
    uint32_t block_bitmap_idx;
    uint32_t all_blocks[140] = {0};
    //  a. 先将前12个直接块存入all_blocks
    while(block_idx < 12)
    {
        all_blocks[block_idx] = inode_to_del->i_sectors[block_idx];
        block_idx++;
    }
    //  b. 如果一级间接块表存在, 将其128个间接块读到all_blocks[12~], 并释放一级间接块表所在的扇区
    if(inode_to_del->i_sectors[12]!=0)
    {
        ide_read(part->my_disk, inode_to_del->i_sectors[12], all_blocks + 12, 1);
        block_cnt = 140;
        //回收一级间接块表占用的扇区
        block_bitmap_idx = inode_to_del->i_sectors[12] - part->sb->data_start_lba;
        ASSERT(block_bitmap_idx > 0);
        bitmap_setbit(&part->block_bitmap, block_bitmap_idx, 0);
        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
    }
    //  c.inode所有的块地址已经收集到all_blocks中, 下面逐个回收
    block_idx = 0;
    while(block_idx < block_cnt)
    {
        if(all_blocks[block_idx]!=0)
        {
            block_bitmap_idx = 0;
            block_bitmap_idx = all_blocks[block_idx] - part->sb->data_start_lba;
            ASSERT(block_bitmap_idx > 0);
            bitmap_setbit(&part->block_bitmap, block_bitmap_idx, 0);
            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
        }
        block_idx++;
    }
    //2.回收该inode所占用的inode
    bitmap_setbit(&part->inode_bitmap, inode_no, 0);
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    inode_close(inode_to_del);
}

