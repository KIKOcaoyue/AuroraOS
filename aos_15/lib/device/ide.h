#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H
#include "stdint.h"
#include "deque.h"
#include "sync.h"
#include "bitmap.h"
#include "stdio.h"
#include "timer.h"
#include "io.h"
#include "debug.h"
#include "memory.h"
#include "superblock.h"


/* 分区表结构 */
struct PARTITION
{
    uint32_t start_lba;                      //起始扇区
    uint32_t sec_cnt;                        //扇区数
    struct DISK* my_disk;                    //分区所属的硬盘
    struct DEQUE_ELEM part_tag;              //队列标记
    char name[8];                            //分区名称
    struct super_block* sb;                  //本分区的超级块
    struct bitmap block_bitmap;              //块位图
    struct bitmap inode_bitmap;              //i结点位图
    struct DEQUE open_inodes;                //本分区打开的i结点队列
};

/* 硬盘结构 */
struct DISK
{
    char name[8];                            //本硬盘名称
    struct IDE_CHANNEL* my_channel;          //此块硬盘归属于哪个ide通道
    uint8_t dev_no;                          //本硬盘是主0, 还是从1
    struct PARTITION prim_parts[4];          //主分区最多4个
    struct PARTITION logic_parts[8];         //逻辑分区数量无限, 但这里只给8个
};

/* ata通道结构 */
struct IDE_CHANNEL
{
    char name[8];                            //本ata通道的名称
    uint16_t port_base;                      //本通道的起始端口号
    uint8_t irq_no;                          //本通道所用的中断号
    struct lock lck;                         //通道锁
    int expecting_intr;                      //表示等待硬盘的中断
    struct semaphore disk_done;              //用于阻塞, 唤醒驱动程序
    struct DISK devices[2];                  //一个通道上连接两个硬盘, 一主一从
};

extern uint8_t channel_cnt;                                                   //按硬盘数计算的通道数
extern struct IDE_CHANNEL channels[2];                                        //有两个ide通道
extern struct DEQUE partition_list;
void ide_init();

#endif 
