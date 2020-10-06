#include "print.h"
#include "init.h"
#include "debug.h"
#include "bitmap.h"
#include "memory.h"
#include "thread.h"
#include "console.h"
#include "keyboard.h"
#include "iobuf.h"
#include "process.h"
#include "syscall.h"
#include "syscallinit.h"
#include "stdio.h"
#include "fs.h"
#include "dir.h"


int prog_a_pid = 0, prog_b_pid = 0;

void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);
int main()
{
    put_str("I am kernel!\n");
    init_all();
    // process_execute(u_prog_a, "user_prog_a");
    // process_execute(u_prog_b, "user_prog_b");
    // intr_enable();
    // console_put_str("main_pid:0x");
    // console_put_int(sys_getpid());
    // console_put_str("\n");
    // thread_start("k_thread_a", 31, k_thread_a, "I am thread_a ");
    // thread_start("k_thread_b", 31, k_thread_b, "I am thread_b ");
    // uint32_t fd = sys_open("/file1", O_RDWR);
    // printf("open /file1, fd:%d\n", fd);
    // char buf[64] = {0};
    // int read_bytes = sys_read(fd, buf, 18);
    // printf("1_ read %d bytes:\n%s\n", read_bytes, buf);

    // memset(buf, 0, 64);
    // read_bytes = sys_read(fd, buf, 6);
    // printf("2_ read %d bytes:\n%s\n", read_bytes, buf);

    // memset(buf, 0, 64);
    // read_bytes = sys_read(fd, buf, 6);
    // printf("3_ read %d bytes:\n%s\n", read_bytes, buf);

    // printf("------------------SEEK_SET 0---------------------\n");
    // sys_lseek(fd, 0, SEEK_SET);

    // memset(buf, 0, 64);
    // read_bytes = sys_read(fd, buf, 24);
    // printf("4_ read %d bytes:\n%s\n", read_bytes, buf);

    // sys_close(fd);

    // printf("/dir1/subdir1 create %s!\n", sys_mkdir("/dir1/subdir1")==0 ? "done" : "failed");
    // printf("/dir1 create %s!\n", sys_mkdir("/dir1")==0 ? "done" : "failed");
    // printf("new /dir1/subdir1 create %s!\n", sys_mkdir("/dir1/subdir1")==0 ? "done" : "failed");
    // int fd = sys_open("/dir1/subdir1/file1", O_CREAT|O_RDWR);
    // if(fd!=-1)
    // {
    //     printf("/dir1/subdir1/file1 create done\n");
    //     sys_write(fd, "Catch me if you can!\n", 21);
    //     sys_lseek(fd, 0, SEEK_SET);
    //     char buf[32] = {0};
    //     sys_read(fd, buf, 21);
    //     printf("/dir1/subdir1/file1 says: \n%s", buf);
    //     sys_close(fd);
    // }

    struct dir* p_dir = sys_opendir("/dir1/subdir1");
    if(p_dir)
    {
        printf("/dir1/subdir1 close done!\ncontent:\n");
        char* type = NULL;
        struct dir_entry* dir_e = NULL;
        while((dir_e=sys_readdir(p_dir)))
        {
            if(dir_e->f_type==FT_REGULAR)
            {
                type = "regular";
            }
            else 
            {
                type = "directory";
            }
            printf("    %s %s\n", type, dir_e->filename);
        }
        if(sys_closedir(p_dir)==0)
        {
            printf("close done!\n");
        }
        else
        {
            printf("close failed!\n");
        }
    }
    else 
    {
        printf("/dir1/subdir1 open failed!\n");
    }

    while(1)
    {
        //console_put_str("Main ");
    }
    return 0;
}

void k_thread_a(void* arg)
{
    char* para = arg;  
    while(1);
}

void k_thread_b(void* arg)
{
    char* para = arg;
    while(1);
}
/*


void u_prog_a(void)
{
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf("prog_a malloc addr:0x%x, 0x%x, 0x%x\n",(int)addr1,(int)addr2,(int)addr3);
    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while(1);
}

void u_prog_b(void)
{
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf("prog_b malloc addr:0x%x, 0x%x, 0x%x\n",(int)addr1,(int)addr2,(int)addr3);
    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while(1);
}
*/
/*
void k_thread_a(void* arg)
{
    char* para = arg;
    void* addr1;
    void* addr2;
    void* addr3;
    void* addr4;
    void* addr5;
    void* addr6;
    void* addr7;
    console_put_str(" thread_a start\n");
    int max = 100;
    while(max-- > 0)
    {
        printf("thread_A max:%d\n",max);
        int size = 128;
        addr1 = sys_malloc(size);
        // console_put_str("addr1: 0x");
        // console_put_int((uint32_t)addr1);
        // console_put_char(' ');
        size *= 2;
        addr2 = sys_malloc(size);
        // console_put_str("addr2: 0x");
        // console_put_int((uint32_t)addr2);
        // console_put_char(' ');
        // size *= 2;
        addr3 = sys_malloc(size);
        // console_put_str("addr3: 0x");
        // console_put_int((uint32_t)addr3);
        // console_put_char(' ');
        sys_free(addr1);
        // console_put_str("addr1: 0x");
        // console_put_int((uint32_t)addr1);
        // console_put_char(' ');
        addr4 = sys_malloc(size);
        // console_put_str("addr4: 0x");
        // console_put_int((uint32_t)addr4);
        // console_put_char(' ');
        size *= 2; size *= 2; size *= 2; size *= 2; size *= 2; size *= 2; size *= 2;
        addr5 = sys_malloc(size);
        // console_put_str("addr5: 0x");
        // console_put_int((uint32_t)addr5);
        // console_put_char(' ');
        addr6 = sys_malloc(size);
        // console_put_str("addr6: 0x");
        // console_put_int((uint32_t)addr6);
        // console_put_char(' ');
        sys_free(addr5);
        // console_put_str("addr5: 0x");
        // console_put_int((uint32_t)addr5);
        // console_put_char(' ');
        size *= 2;
        addr7 = sys_malloc(size);
        // console_put_str("addr7: 0x");
        // console_put_int((uint32_t)addr7);
        // console_put_char(' ');
        sys_free(addr6);
        // console_put_str("addr6: 0x");
        // console_put_int((uint32_t)addr6);
        // console_put_char(' ');
        sys_free(addr7);
        // console_put_str("addr7: 0x");
        // console_put_int((uint32_t)addr7);
        // console_put_char(' ');
        sys_free(addr2);
        // console_put_str("addr2: 0x");
        // console_put_int((uint32_t)addr2);
        // console_put_char(' ');
        sys_free(addr3);
        // console_put_str("addr3: 0x");
        // console_put_int((uint32_t)addr3);
        // console_put_char(' ');
        sys_free(addr4);
        // console_put_str("addr4: 0x");
        // console_put_int((uint32_t)addr4);
        // console_put_char(' ');
    }
    console_put_str(" thread_a end\n");
    while(1);
}

void k_thread_b(void* arg)
{
    char* para = arg;
    void* addr1;
    void* addr2;
    void* addr3;
    void* addr4;
    void* addr5;
    void* addr6;
    void* addr7;
    void* addr8;
    void* addr9;
    console_put_str(" thread_b start\n");
    int max = 100;
    while(max-- > 0)
    {
        printf("thread_B max:%d\n",max);
        int size = 9;
        addr1 = sys_malloc(size);
        // console_put_str("KB_addr1: 0x");
        // console_put_int((uint32_t)addr1);
        // console_put_char(' ');
        size *= 2;
        addr2 = sys_malloc(size);
        // console_put_str("KB_addr2: 0x");
        // console_put_int((uint32_t)addr2);
        // console_put_char(' ');
        size *= 2;
        sys_free(addr2);
        // console_put_str("KB_addr2: 0x");
        // console_put_int((uint32_t)addr2);
        // console_put_char(' ');
        addr3 = sys_malloc(size);
        // console_put_str("KB_addr3: 0x");
        // console_put_int((uint32_t)addr3);
        // console_put_char(' ');
        sys_free(addr1);
        // console_put_str("KB_addr1: 0x");
        // console_put_int((uint32_t)addr1);
        // console_put_char(' ');
        addr4 = sys_malloc(size);
        // console_put_str("KB_addr4: 0x");
        // console_put_int((uint32_t)addr4);
        // console_put_char(' ');
        addr5 = sys_malloc(size);
        // console_put_str("KB_addr5: 0x");
        // console_put_int((uint32_t)addr5);
        // console_put_char(' ');
        addr6 = sys_malloc(size);
        // console_put_str("KB_addr6: 0x");
        // console_put_int((uint32_t)addr6);
        // console_put_char(' ');
        sys_free(addr5);
        // console_put_str("KB_addr5: 0x");
        // console_put_int((uint32_t)addr5);
        // console_put_char(' ');
        size *= 2;
        addr7 = sys_malloc(size);
        // console_put_str("KB_addr7: 0x");
        // console_put_int((uint32_t)addr7);
        // console_put_char(' ');
        sys_free(addr6);
        // console_put_str("KB_addr6: 0x");
        // console_put_int((uint32_t)addr6);
        // console_put_char(' ');
        sys_free(addr7);
        // console_put_str("KB_addr7: 0x");
        // console_put_int((uint32_t)addr7);
        // console_put_char(' ');
        sys_free(addr3);
        // console_put_str("KB_addr3: 0x");
        // console_put_int((uint32_t)addr3);
        // console_put_char(' ');
        sys_free(addr4);
        // console_put_str("KB_addr4: 0x");
        // console_put_int((uint32_t)addr4);
        // console_put_char(' ');

        size *= 2; size *= 2; size *= 2;
        addr1 = sys_malloc(size);
        // console_put_str("KB_addr1: 0x");
        // console_put_int((uint32_t)addr1);
        // console_put_char(' ');
        addr2 = sys_malloc(size);
        //console_put_str("KB_addr2: 0x");
        //console_put_int((uint32_t)addr2);
        //console_put_char(' ');
        addr3 = sys_malloc(size);
        //console_put_str("KB_addr3: 0x");
        //console_put_int((uint32_t)addr3);
        //console_put_char(' ');
        addr4 = sys_malloc(size);
        //console_put_str("KB_addr4: 0x");
        //console_put_int((uint32_t)addr4);
        //console_put_char(' ');
        addr5 = sys_malloc(size);
        //console_put_str("KB_addr5: 0x");
        //console_put_int((uint32_t)addr5);
        //console_put_char(' ');
        addr6 = sys_malloc(size);
        //console_put_str("KB_addr6: 0x");
        //console_put_int((uint32_t)addr6);
        //console_put_char(' ');
        addr7 = sys_malloc(size);
        //console_put_str("KB_addr7: 0x");
        //console_put_int((uint32_t)addr7);
        //console_put_char(' ');
        addr8 = sys_malloc(size);
        //console_put_str("KB_addr8: 0x");
        //console_put_int((uint32_t)addr8);
        //console_put_char(' ');
        addr9 = sys_malloc(size);
        //console_put_str("KB_addr9: 0x");
        //console_put_int((uint32_t)addr9);
        //console_put_char(' ');
        sys_free(addr1);
        //console_put_str("KB_addr1: 0x");
        //console_put_int((uint32_t)addr1);
        //console_put_char(' ');
        sys_free(addr2);
        //console_put_str("KB_addr2: 0x");
        //console_put_int((uint32_t)addr2);
        //console_put_char(' ');
        sys_free(addr3);
        //console_put_str("KB_addr3: 0x");
        //console_put_int((uint32_t)addr3);
        //console_put_char(' ');
        sys_free(addr4);
        //console_put_str("KB_addr4: 0x");
        //console_put_int((uint32_t)addr4);
        //console_put_char(' ');
        sys_free(addr5);
        //console_put_str("KB_addr5: 0x");
        //console_put_int((uint32_t)addr5);
        //console_put_char(' ');
        sys_free(addr6);
        //console_put_str("KB_addr6: 0x");
        //console_put_int((uint32_t)addr6);
        //console_put_char(' ');
        sys_free(addr7);
        //console_put_str("KB_addr7: 0x");
        //console_put_int((uint32_t)addr7);
        //console_put_char(' ');
        sys_free(addr8);
        //console_put_str("KB_addr8: 0x");
        //console_put_int((uint32_t)addr8);
        //console_put_char(' ');
        sys_free(addr9);
        //console_put_str("KB_addr9: 0x");
        //console_put_int((uint32_t)addr9);
        //console_put_char(' ');
    }
    console_put_str(" thread_b end\n");
    while(1);
}
*/

void u_prog_a(void)
{
    printf("I am %s, u_prog_a_pid:0x%x%c","u_prog_a",getpid(),'\n');
    while(1)
    {
        //printf("u_prog_a ");
    }
}

void u_prog_b(void)
{
    printf("I am %s, u_prog_b_pid:%d%c","u_prog_b",getpid(),'\n');
    while(1)
    {
        //printf("u_prog_b ");
    }
}