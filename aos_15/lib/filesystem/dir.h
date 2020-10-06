#ifndef __FILESYSTEM_DIR_H
#define __FILESYSTEM_DIR_H

#include "stdint.h"
#include "fs.h"
#include "inode.h"

#define MAX_FILE_NAME_LEN 16

struct dir 
{
    struct inode* inode;
    uint32_t dir_pos;               //在目录中的偏移
    uint8_t dir_buf[512];           //目录的数据缓存
};

struct dir_entry
{
    char filename[MAX_FILE_NAME_LEN];
    uint32_t i_no;
    enum file_types f_type;
};

extern struct dir root_dir;

#endif 