#include "config.h"
#include "dev_manage.h"
#include "dev_manage_api.h"

int dev_mount_api(dev_io *io, int major, void *parm)
{
    if(io == NULL)
        return PTR_NULL;

    if(major == 0xffff)  //自动获取一个空设备号
       major = get_avail_major();

    if(major >= MAX_DEV)
       return MAJOR_ERR;

    return dev_mount(io, major, parm);
}

int dev_umount_api(dev_hd *hd)
{
    if(hd == NULL)
        return PTR_NULL;

    return dev_umount(hd->dev_io, hd->dev_major);
}

int dev_open_api(dev_hd **hd, const char *dev_name)
{
    if(hd == NULL)
        return PTR_NULL;

    return dev_open(hd, dev_name);
}

char *get_dev_name_by_hd(dev_hd *hd)
{
    if(hd == NULL)
        return NULL;

    return ((dev_io *)(hd->dev_io))->dev_name;
}


int dev_read_api(dev_hd *hd, unsigned int addr,  void *buffer,  int buffer_len)
{
    if((hd == NULL) || (buffer == 0))
        return PTR_NULL;

    dev_io *io = hd->dev_io;
    if(io->dev_type == BLOCK_DEV)
    {
        if(io->read)
        {
            hd->mutu   = 1;
            buffer_len = io->read(addr, buffer, buffer_len);
            hd->mutu   = 0;
            return buffer_len;
        }
        else
            return IO_ERR;
    }
    else if(io->dev_name == CHARACTER_DEV)
    {
        if(hd->r_buff)  //先从缓存中找是否有数据
        {
            int read_size;
            hd->mutu   = 1;
            read_size  = read_cbuff(hd->r_buff, buffer, buffer_len);
            hd->mutu   = 0;
            if(read_size)
                return read_size;
        }

        if(io->read)
        {
            hd->mutu    = 1;
            buffer_len  =  io->read(0, buffer, buffer_len);
            hd->mutu    = 0;
            return buffer_len;
        }
    }
    return 0;
}

int dev_write_api(dev_hd *hd, unsigned int addr, void *buffer, int buffer_len)
{
    if((hd == NULL) || (buffer == NULL))
        return PTR_NULL;

    dev_io *io = hd->dev_io;

    if(io->dev_type == BLOCK_DEV)
    {
        if(io->write)
        {
            hd->mutu   = 1;
            buffer_len = io->write(addr, buffer, buffer_len);
            hd->mutu   = 0;
            return buffer_len;
        }
        else
            return IO_ERR;
    }
    else if(io->dev_type == CHARACTER_DEV)
    {
        if(io->write)
        {
            hd->mutu   = 1;
            buffer_len = io->write(0, buffer, buffer_len);
            hd->mutu   = 0;
            return buffer_len;
        }
        else
            return IO_ERR;
    }
    return 0;
}

int dev_ioctrl_api(dev_hd *hd, int cmd, void *parm)
{
    int res = 0;
    if(hd == NULL)
        return PTR_NULL;

    dev_io *io = hd->dev_io;

    if(io->ioctrl)
    {
        hd->mutu = 1;
        res      = io->ioctrl(cmd, parm);
        hd->mutu = 0;
    }

    return res;
}

//设备管理测试
#if 1
#include "task_manage.h"
int dev_mg_task_run(void *arg)
{
		dev_event_callback_run();
		return 0;
}

int dev_mg_task_init()
{
		int res;
		res = dev_manage_init();
		return res;
}
int dev_mg_task_exit()
{
		return 0;
}
register_task_list dev_mg_task =
{
		.name			 = (char *)"dev_manage",
		.task_func = dev_mg_task_run,
		.task_init = dev_mg_task_init,
		.task_exit = dev_mg_task_exit,
		.run_flag  = 1,  //上电自启
		.q_msg	   = NULL,
};
#ifdef CMD_LINE
#include "cmd.h"
const char *dev_cmd_table[]=
{
		"read",  //打印当前运行的命令
		"open"
};

static dev_hd *sd_dev = NULL;
static int dev_cmd_deal(int cmd_index, char *arg)
{
		switch(cmd_index) {
				case 0:
						{
								u8 buffer[512];
								int addr = str_to_int(arg);
								int res;
								memset(buffer, 0x00, 512);
								res = dev_read_api(sd_dev,addr, buffer, 1);	
								if(res != 1)
										uprintf("dev read err :%d\n\r", res);
								else
									printf_buff(buffer, 512);
						}
						break;
				case 1:
						uprintf("open dev %s enter\n\r", arg);
						return dev_open_api(&sd_dev, arg);  
				break;
				default:
						return 1;
		}
		return 0;
}

static int dev_do(char **arg, int arg_num)
{
		char dev_cmd_cnt;; 
		while(arg_num--)		{
				for(dev_cmd_cnt = 0; dev_cmd_cnt<sizeof(dev_cmd_table)/sizeof(dev_cmd_table[0]); dev_cmd_cnt++) {
						if(strcmp(*arg,dev_cmd_table[dev_cmd_cnt]) == 0)	{
								if(dev_cmd_deal(dev_cmd_cnt, *(arg+1)))
										return 1;
						}
				}
		}
}
REGISTER_CMD(dev, dev_do, "设备管理类型命令");
#endif
#endif




