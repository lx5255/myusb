#ifndef _DEV_MANAGE_
#define _DEV_MANAGE_

#define MAX_DEV         30      //最多管理多少个设备
#define USE_MALLOC      1       //使用内存分配
#define EVENT_CALLBACK  1       //回调数据到来或其他事件，是否需要回调
#define DEV_MG_DEBUG


#if EVENT_CALLBACK
#define MSG_POOL_SIZE    30
#endif
#define R_BUFF_MAX      512     //读取缓存的大小

#ifdef DEV_MG_DEBUG
#define dev_printf	uprintf
#else
#define dev_printf(...)
#endif

#if EVENT_CALLBACK
typedef struct dev_msg_struct
{
    unsigned int  *pool_start;
    unsigned int  *pool_end;
    unsigned int  *read_ptr;
    unsigned int  *write_ptr;
    unsigned int  msg_cnt;
}dev_msg_struct;
#endif

#define BLOCK_DEV       1
#define CHARACTER_DEV   2
typedef struct dev_io
{
   char *dev_name;
   char boot_monut;  //上电挂载标志  1上电由设备管理模块挂载， 0,：由用户自己挂载
   char dev_type;
   //addr 在块设备中作为数据的寻址，确定数据位置， 在字符型设备中，应该被忽略
   int (*read)(long int addr, void *buffer, int buffer_len);
   int (*write)(long int addr, void *buffer, int buffer_len);
   int (*ioctrl)(int cmd, void *parm);
   int (*mount)(int major, void *parm);
   int (*umount)();
 //  void (*register_callbcak_func)(int (*read_cb)(unsigned char *buffer, int buffer_len));
}dev_io;



typedef struct dev_hd
{
    void *dev_io;
    int  dev_major;
    void *r_buff;
    int (*data_read_callback)(void *buffer, int buffer_len);
#if EVENT_CALLBACK
    dev_msg_struct dev_msg;
#endif
    // void *w_buff;
    //int dev_type;
    char mutu;
}dev_hd;

enum
{
   CROSS_BORDER = 0x01,
   PTR_NULL,
   BUFF_LACK,
   ALLOC_ERR,
   NO_MSG,
   MAJOR_ERR,
   IO_ERR,
}DEV_ERR_NUM;

#if EVENT_CALLBACK
enum
{
    DATA_COMING = 0x01,
    SYS_EVENT_END,  //系统消息末尾
}DEV_MSG;
int post_dev_mg_msg(int major, int event, int data);
#endif

int dev_mount(dev_io *io, int major, void *parm);
int dev_umount(dev_io *io, int major);
int dev_open(dev_hd **hd, const char *dev_name);
int dev_manage_read_callback(int major, void *buffer, int buffer_len);

#define register_dev_io __attribute__((section(".dev_list")))  dev_io
#endif
