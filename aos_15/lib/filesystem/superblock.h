#ifndef __FILESYSTEM_SUPERBLOCK_H
#define __FILESYSTEM_SUPERBLOCK_H

#include "stdint.h"

struct super_block
{
    uint32_t magic;                    //用来标识文件系统类型
    uint32_t sec_cnt;                  //本分区总共的扇区数
    uint32_t inode_cnt;                //本分区中inode数量
    uint32_t part_lba_base;            //本分区起始lba地址
    uint32_t block_bitmap_lba;         //块位图本身起始扇区地址
    uint32_t block_bitmap_sects;       //扇区位图本身占用的扇区数量
    uint32_t inode_bitmap_lba;         //i结点位图起始扇区lba地址
    uint32_t inode_bitmap_sects;       //i结点位图占用的扇区数量
    uint32_t inode_table_lba;          //i结点表起始扇区lba地址
    uint32_t inode_table_sects;        //i结点表占用的扇区数量
    uint32_t data_start_lba;           //数据起始扇区lba地址
    uint32_t root_inode_no;            //根目录所在的i结点号
    uint32_t dir_entry_size;           //目录项大小
    uint8_t pad[460];                  //加上460字节, 凑够512字节1扇区大小
}__attribute__((packed));

#endif 