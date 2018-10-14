#ifndef _DEV_MANAGE_
#define _DEV_MANAGE_

#define MAX_DEV         30      //��������ٸ��豸
#define USE_MALLOC      1       //ʹ���ڴ����
#define EVENT_CALLBACK  1       //�ص����ݵ����������¼����Ƿ���Ҫ�ص�
#define DEV_MG_DEBUG


#if EVENT_CALLBACK
#define MSG_POOL_SIZE    30
#endif
#define R_BUFF_MAX      512     //��ȡ����Ĵ�С

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
   char boot_monut;  //�ϵ���ر�־  1�ϵ����豸����ģ����أ� 0,�����û��Լ�����
   char dev_type;
   //addr �ڿ��豸����Ϊ���ݵ�Ѱַ��ȷ������λ�ã� ���ַ����豸�У�Ӧ�ñ�����
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
    SYS_EVENT_END,  //ϵͳ��Ϣĩβ
}DEV_MSG;
int post_dev_mg_msg(int major, int event, int data);
#endif

int dev_mount(dev_io *io, int major, void *parm);
int dev_umount(dev_io *io, int major);
int dev_open(dev_hd **hd, const char *dev_name);
int dev_manage_read_callback(int major, void *buffer, int buffer_len);

#define register_dev_io __attribute__((section(".dev_list")))  dev_io
#endif
