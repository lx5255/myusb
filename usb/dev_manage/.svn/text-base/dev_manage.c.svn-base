#include "config.h"
#include "dev_manage.h"
#include "dev_manage_api.h"
#include "c_buff.h"

#if (USE_MALLOC == 1)
//#include "malloc.h"
static dev_hd *dev_hd_group = NULL;
#else
static dev_hd dev_hd_group[MAX_DEV];
#endif

#if EVENT_CALLBACK
static unsigned int sys_msg_pool[MSG_POOL_SIZE];
static dev_msg_struct dev_sys_msg;
static int dev_msg_pool_init(dev_msg_struct *dev_msg, void *pool_buffer,  int buffer_len);
    #if !USE_MALLOC
static int msg_pool_group[MAX_DEV][MSG_POOL_SIZE];
    #endif
#endif
static void boot_monut_dev();
/*----------------------------------------------------------------------------*/
/** @brief: 初始化设备管理模块
    @param: null
    @return: 错误类型
    @note:
*/
/*----------------------------------------------------------------------------*/
int dev_manage_init()
{
#if (USE_MALLOC == 1)
   dev_hd_group = (dev_hd *)malloc(sizeof(dev_hd)*MAX_DEV);
   if(dev_hd_group == NULL)
       return ALLOC_ERR;
#endif
    memset(dev_hd_group, 0x00, sizeof(dev_hd)*MAX_DEV);

#if EVENT_CALLBACK
    dev_msg_pool_init(&dev_sys_msg, (void *)sys_msg_pool, sizeof(sys_msg_pool));
#endif
    boot_monut_dev();  //挂载需要上电启动的设备
		return 0;
}
/*----------------------------------------------------------------------------*/
/** @brief: 获取一个空的设备号
    @param: null
    @return: 设备号
    @note:
*/
/*----------------------------------------------------------------------------*/
int get_avail_major()
{
		int major;
    for(major = 0; major<MAX_DEV; major++)
    {
       if(dev_hd_group[major].dev_io == NULL)
       {
            return major;
       }
    }
    return 0xffff;
}
/*----------------------------------------------------------------------------*/
/** @brief: 设备挂载
    @param: io:设备操作IO  major:设备号   parm:挂载参数
    @return: 错误类型
    @note:
*/
/*----------------------------------------------------------------------------*/
int dev_mount(dev_io *io, int major, void *parm)
{
    int res;

    if(dev_hd_group[major].dev_io) //当前设备号已被挂载
        return  MAJOR_ERR;

    res = io->mount(major, 0);
		dev_printf("io mount %d\n\r", res);
    if(!res)  //挂载成功
    {
        int *msg_pool;
        dev_hd *dev     = &dev_hd_group[major];
#if EVENT_CALLBACK
    #if USE_MALLOC
        msg_pool        = (int *)malloc(sizeof(int)*MSG_POOL_SIZE);
        if(msg_pool == NULL)
            return ALLOC_ERR;
    #else
        msg_pool        = &msg_pool_group[major];
    #endif
        dev_msg_pool_init(&(dev->dev_msg), (void *)msg_pool, sizeof(int)*MSG_POOL_SIZE);
#endif
        dev->dev_io     = (void *)io;
        dev->dev_major  = major;
        dev->mutu       = 0;
   }
    return res;
}
/*----------------------------------------------------------------------------*/
/** @brief: 设备卸载
    @param: io 设备操作io
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
int dev_umount(dev_io *io, int major)
{
    int res;
   res = io->umount();
   if(!res)
   {
       dev_hd *dev = &dev_hd_group[major];
#if EVENT_CALLBACK
    #if USE_MALLOC
       free(dev->dev_msg.pool_start);
    #else
       memset(dev->dev_msg.pool_start, 0x00, sizeof(int)*MSG_POOL_SIZE);
    #endif
#endif
       if(dev->r_buff)
           free_cbuff(dev->r_buff);

       memset(dev, 0x00, sizeof(dev_hd));
   }
}

int dev_open(dev_hd **hd, const char *dev_name)
{
   dev_hd *hd_ptr = dev_hd_group;
   for(; (hd_ptr - dev_hd_group)/sizeof(dev_hd) < MAX_DEV; hd_ptr++)
   {
				dev_printf("hd_ptr: %x\n\r", hd_ptr);
        if(hd_ptr->dev_io)
        {
						dev_printf("list dev name:%s\n\r", ((dev_io *)hd_ptr->dev_io)->dev_name);
            if(strcmp(dev_name, ((dev_io *)hd_ptr->dev_io)->dev_name) == 0)  //设备名比对成功
            {
								dev_printf("dev open success\n\r");
                *hd = hd_ptr;
                return 0;
            }
        }
   }
   *hd = NULL;
    return 1;
}

int get_hd_by_mojor(int major)
{

}

extern _dev_list_start, _dev_list_end;
dev_io *get_dev_io_byname(const char *dev_name)
{
    dev_io *dev_list_ptr = &_dev_list_start;
    for(; dev_list_ptr <&_dev_list_end; dev_list_ptr++)
    {
        if(strcmp(dev_list_ptr->dev_name, dev_name) == 0)
        {
             return dev_list_ptr;
        }
    }

    return NULL;
}

static void boot_monut_dev()
{
   dev_io *dev_list_ptr = &_dev_list_start;
   for(; dev_list_ptr <&_dev_list_end; dev_list_ptr++)
   {
				dev_printf("mount dev:%x\n\r", dev_list_ptr); 
        if(dev_list_ptr->boot_monut == 1)  //需要上电挂载
        {
            if(dev_list_ptr->mount)
            {
                int major;
								int res;
                major = get_avail_major();  //获取一个空的设备号
                if(major == 0xffff)  //已经挂满了设备
                    return;
                if(dev_mount(dev_list_ptr, major, 0) == 0)
										dev_printf("mount %s success\n\r", dev_list_ptr->dev_name);
           }
        }
   }
}

int dev_manage_read_callback(int major, void *buffer, int buffer_len)
{
    if(major >= MAX_DEV)
        return CROSS_BORDER;

    if(buffer_len == 0)
        return 0;

    if(dev_hd_group[major].dev_io == NULL)  //当前设备号没有设备
        return PTR_NULL;

    if(dev_hd_group[major].r_buff == NULL)
    {
        dev_hd_group[major].r_buff = (void *)new_cbuff(R_BUFF_MAX);
        if(dev_hd_group[major].r_buff == NULL)
            return PTR_NULL;
    }

    c_buff_struct *cbuff = (c_buff_struct *)dev_hd_group[major].r_buff;
    int remain_size = get_cbuff_remain(cbuff);
    if(remain_size < buffer_len)  //cbuff的空间不够放下这么多数据
        return BUFF_LACK;

    buffer_len = write_cbuff(cbuff, buffer, buffer_len);
#if EVENT_CALLBACK
    post_dev_mg_msg(DATA_COMING, major, buffer_len);
#endif

    return 0;
}

int get_dev_msg(dev_hd *hd, unsigned int *event, unsigned int *data)
{
#if EVENT_CALLBACK
    if(hd == NULL)
        return PTR_NULL;

    if(hd->dev_msg.msg_cnt)
    {
        DIS_CPU_INT
        if(event)
            *event = *(hd->dev_msg.read_ptr++);
        if(data)
            *data  = *(hd->dev_msg.read_ptr++);
        hd->dev_msg.msg_cnt--;
        EN_CPU_INT
        return 0;
    }
#endif
    return NO_MSG;
}
#if EVENT_CALLBACK
static int dev_msg_pool_init(dev_msg_struct *dev_msg, void *pool_buffer,  int buffer_len)
{
    dev_msg->pool_start  = pool_buffer;
    dev_msg->pool_end    = pool_buffer + buffer_len;
    dev_msg->read_ptr    = pool_buffer;
    dev_msg->write_ptr   = pool_buffer;
    dev_msg->msg_cnt     = 0;
}

static int pend_dev_mg_msg(int *major, int *event, int *data)
{
   if(dev_sys_msg.msg_cnt)
   {
        if(dev_sys_msg.read_ptr > dev_sys_msg.pool_end)
        {
            dev_sys_msg.read_ptr = dev_sys_msg.pool_start;
        }

        DIS_CPU_INT
        *major = *(dev_sys_msg.read_ptr++);
        *event = *(dev_sys_msg.read_ptr++);
        *data  = *(dev_sys_msg.read_ptr++);
        dev_sys_msg.msg_cnt--;
        EN_CPU_INT
        return 0;
   }
  else
  {
        event = 0x00;
        return 1;
  }
}

int post_dev_mg_msg(int major, int event, int data)
{
    if(event < SYS_EVENT_END)  //是系统事件
    {
        if(dev_sys_msg.write_ptr > dev_sys_msg.pool_end - 3)
        {
            dev_sys_msg.write_ptr = dev_sys_msg.pool_start;
        }
        if(dev_sys_msg.write_ptr == dev_sys_msg.read_ptr)  //消息池满了，丢弃
            return BUFF_LACK;

        DIS_CPU_INT
        *(dev_sys_msg.write_ptr++) = major;
        *(dev_sys_msg.write_ptr++) = event;
        *(dev_sys_msg.write_ptr++) = data;
        dev_sys_msg.msg_cnt++;
        EN_CPU_INT
    }
    else
    {
        dev_msg_struct *dev_msg = &dev_hd_group[major].dev_msg;
        if(dev_msg->pool_start == NULL)
            return PTR_NULL;

        if(dev_msg->write_ptr > dev_msg->pool_end - 2)
        {
            dev_msg->write_ptr = dev_msg->pool_start;
        }
        if(dev_msg->write_ptr == dev_msg->read_ptr)
            return BUFF_LACK;

        DIS_CPU_INT
        *(dev_msg->write_ptr++) = event;
        *(dev_msg->write_ptr++) = data;
        dev_msg->msg_cnt++;
        EN_CPU_INT
    }
}

void dev_event_callback_run()
{
   int event, major, data;
   int res;

   res = pend_dev_mg_msg(&major, &event, &data);
   switch(event)
   {
       case DATA_COMING:
           if(dev_hd_group[major].data_read_callback)
           {
              unsigned char buffer[R_BUFF_MAX];
              int data_size = read_cbuff(dev_hd_group[major].r_buff, buffer, R_BUFF_MAX);
              dev_hd_group[major].data_read_callback(buffer, data_size);
           }
           break;
        default:
           break;
   }
}
#endif

