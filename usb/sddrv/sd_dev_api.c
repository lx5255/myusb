#include "sd_drv.h"
#include "sd_dev_api.h"
#include "dev_manage.h"
static int sd_major = 0xffff; 
int sd_dev_mount(int major, void *parm)
{
	sd_major = major;
  return sd_init();  //初始化SD卡
}

int sd_dev_umount()
{
		if(get_sd_busy())  //SD卡在忙 无法卸载
				return 1; 

		sd_exit();
		return 0; 
}

int sd_read_api(unsigned int block_addr, void *buffer, int len)
{
		int res, block_cnt = 0;
		while(len--)		
		{
				res = sd_read_block(block_addr, (u8 *)buffer, 1);	
				if(res)
						return block_addr;

				block_cnt++;
		}
		return block_cnt;
}

int sd_write_api(unsigned int block_addr, void *buffer, int len)
{
		int res, block_cnt = 0;
		while(len--)		
		{
				res = sd_write_block(block_addr, (u8 *)buffer, 1);	
				if(res)
						return block_addr;

				block_addr++;
		}
		return block_addr;
}

int sd_ioctrl_api(int cmd, void *parm)
{
		switch(cmd)
		{
				default:
						return 1;
		}
}
//注册设备io
register_dev_io sd_dev_io =
{
		.dev_name		= "sd",
		.boot_monut = 1,
		.dev_type		= BLOCK_DEV,
		.read				= sd_read_api,
		.write			= sd_write_api,
		.mount			= sd_dev_mount,
		.umount			= sd_dev_umount,
		.ioctrl			= sd_ioctrl_api,
};

