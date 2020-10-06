#ifndef __FILESYSTEM_FILE_H
#define __FILESYSTEM_FILE_H

#include "stdint.h"
#include "dir.h"
struct file
{
    uint32_t fd_pos;                              //记录当前文件操作的偏移地址, 以0为起始, 最大为文件大小-1
    uint32_t fd_flag;                             //文件操作标识, 例如O_RDONLY
    struct inode* fd_inode;                       //指向part->open_inodes中的inode
};

/* 标准输入输出描述符 */
enum std_fd
{
    stdin_no,
    stdout_no, 
    stderr_no
};

/* 位图类型 */
enum bitmap_type
{
    INODE_BITMAP,                                  //inode位图
    BLOCK_BITMAP                                   //块位图
};

#define MAX_FILE_OPEN 32

extern struct file file_table[MAX_FILE_OPEN];

int32_t file_read(struct file* file, void* buf, uint32_t count);
int32_t file_write(struct file* file, const void* buf, uint32_t count);
int32_t file_close(struct file* file);
int32_t file_open(uint32_t inode_no, uint8_t flag);
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag);

#endif 