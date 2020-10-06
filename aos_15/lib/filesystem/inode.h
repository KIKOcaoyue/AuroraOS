#ifndef __FILESYSTEM_INODE_H
#define __FILESYSTEM_INODE_H

#include "stdint.h"
#include "deque.h"
#include "ide.h"

struct inode
{
    uint32_t i_no;                  //inode编号
    uint32_t i_size;                //inode是文件时, i_size表示文件的大小. inode是目录时, i_size指该目录下所有目录项大小之和
    uint32_t i_open_cnts;           //记录此文件被打开的次数
    bool write_deny;                //写文件不能并行, 进程写文件前检查此标识
    uint32_t i_sectors[13];         //i_sectors[0~11]是直接块, 12用来存储一级间接块指针
    struct DEQUE_ELEM inode_tag;
};

void inode_init(uint32_t inode_no, struct inode* new_inode);
void inode_close(struct inode* inode);
struct inode* inode_open(struct PARTITION* part, uint32_t inode_no);
void inode_sync(struct PARTITION* part, struct inode* inode, void* io_buf);


#endif 