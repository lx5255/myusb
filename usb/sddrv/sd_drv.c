#include "config.h"
#include "s3c2440_sdio.h"
#include "sd_drv.h"
#include "s3c2440_uart.h"
//#include "stdlib.h"

const u8 stdio_cmd_table[][5]= //cmd类型配置表{cmd, stopcmd withdata, longrsp, waitrsp}
{
	{0x00, 0x0, 0x0, 0x0 ,0x0},//0
	{0x02, 0x0, 0x0, 0x1 ,0x1},//1
	{0x03, 0x0, 0x0, 0x0 ,0x1},//2
	{0x07, 0x0, 0x0, 0x0 ,0x1},//3
	{0x09, 0x0, 0x0, 0x01,0x01},//4
	{0x0a, 0x0, 0x0, 0x01,0x01},//5
	{0x0b, 0x0, 0x0, 0x0, 0x01},//6
	{0x0c, 0x01,0x0, 0x0, 0x01},//7
	{0x0e, 0x0, 0x0, 0x0, 0x01},//8
	{0x0f, 0x0, 0x0, 0x0, 0x00},//9
	{0x10, 0x0, 0x0, 0x0, 0x01},//10
	{0x11, 0x0, 0x1, 0x0, 0x01},//11
	{0x12, 0x0, 0x1, 0x0, 0x01},//12
	{0x18, 0x0, 0x01,0x0, 0x01},//13
	{0x19, 0x0, 0x01,0x0, 0x01},//14
	{0x1b, 0x0, 0x0, 0x0 ,0x01},//15
	{0x1c, 0x0, 0x0, 0x0 ,0x01},//16
	{0x1d, 0x0, 0x0, 0x0 ,0x01},//17
	{0x1e, 0x0, 0x0, 0x0 ,0x01},//18
	{0x20, 0x0, 0x0, 0x0 ,0x01},//19
	{0x21, 0x0, 0x0, 0x0 ,0x01},//20
	{0x26, 0x0, 0x0, 0x0 ,0x01},//21
	{0x37, 0x0, 0x0, 0x0 ,0x01},//22
	{0x29, 0x0, 0x0, 0x0 ,0x01},//23
	{0x8,  0x0, 0x0, 0x0 ,0x01},//24
	{0x1,  0x0, 0x0, 0x0 ,0x01},//25
	{0x6,  0x0, 0x0, 0x0 ,0x01},//26
};

static u8 *sd_write_buff = NULL;
#if (SD_MALLOC==0)
static u8 sd_write_buffer[SD_BLOCK_SIZE];
#if WIN_CACHE
static u8 sd_win_cache[3][SD_BLOCK_SIZE];
#endif
#endif

static sdio_handle sdio_hd;
//static void analysis_cid(u32 const  *cid);
static sd_handle sd_hd =
{
	.init_flag = 0,
};
static sd_win write_win[2];
static sd_win read_win;

static u8 load_cmd(sdio_handle *hd, u8 cmd, u32 arg)
{
    if(cmd >=sizeof(stdio_cmd_table)/sizeof( stdio_cmd_table[0]))
        return 1;

    hd->cmd.cmd					= stdio_cmd_table[cmd][0];   
    hd->cmd.abortcmd		= stdio_cmd_table[cmd][1];
    hd->cmd.withdata		= stdio_cmd_table[cmd][2];
    hd->cmd.rsp_type		= stdio_cmd_table[cmd][3] + stdio_cmd_table[cmd][4];
   // sd_printf("cmd %d cmd type :%d  %d %d",stdio_cmd_table[cmd][0],  sdio_hd.cmd.rsp_type, stdio_cmd_table[cmd][3] , stdio_cmd_table[cmd][4]);
   // sd_printf("withdata %d",  hd->cmd.withdata);
    hd->cmd.arg = arg;
    return 0;
}
/*----------------------------------------------------------------
**	函数	：void delay(u32 i)
**	功能	：延时一段时间
**	参数	：i ：延时时间
**	返回值：无
**	备注	：无
-------------------------------------------------------------------*/
static u8 sd_send_cmd(u8 cmd, u32 arg)
{
		u8 res;
		while(check_sdio_busy()) //等待接收完成
		{
				delay(10);
		}
//		sd_printf("1\n");    
		res = load_cmd(&sdio_hd, cmd, arg);
		if(res)
				return  1;

//		sd_printf("2\n");    

		res = sdio_cmd_send(&sdio_hd);
		if(res)
		{
				sd_printf("sd cmd err:%d\n", res);
				return 2;
		}
//		sd_printf("3\n");    
		return 0;
}


static u8  analysis_CSD(u32 *csd)
{
     u32 c_size;
     u32 size_mult;
     u8 sd_type = get_bit_value(csd, 126, 1);
     sd_hd.data_tansfer = get_bit_value(csd, 96, 8)>>1;
     sd_hd.block_size = 0x0001<<get_bit_value(csd,80,4);
     if(!get_bit_value(csd, 96, 8))
     {
        c_size = get_bit_value(csd,62,12);
        size_mult = get_bit_value(csd,47,3);
        sd_hd.block_num = (c_size+1)*(0x01<<(size_mult+2));
        sd_hd.sd_size =sd_hd.block_num /1024 * sd_hd.block_size;
     }
     else
     {
         c_size = get_bit_value(csd,48,22);     
         sd_hd.sd_size =(c_size+1)<<9;
         sd_hd.block_num = sd_hd.sd_size*2;///sd_hd.block_size;
     }
    
     sd_printf("c_size:%d  size_mult:%d\n\r", c_size, size_mult);
     
    

     sd_printf("tansfer:%dMbit/s block s:%d  bnum:%d\n\r",\
     sd_hd.data_tansfer, sd_hd.block_size, sd_hd.block_num);
     sd_printf("sd_size:%d\n\r",sd_hd.sd_size);
     return 0;
}

#if WIN_CACHE
static bool write_cache_num = 0;
u16 sd_write(u32 block_num, u16 offset, u8 *buff, u16 buff_len)
{
		u8 res;
		if(buff_len > (SD_BLOCK_SIZE - offset)) //超出块边界
		{
				buff_len = SD_BLOCK_SIZE - offset;
		}

		if(block_num != write_win[write_cache_num].block_num) //不是当前写cache
		{

				write_cache_num = !write_cache_num; //切换cache

				if(block_num != write_win[write_cache_num].block_num) //也不是另一块写cache的数据，需要先取数据
				{
						while(check_sdio_busy()) //操作这块cache之前 确保之前数据已经写入完成
						{
								delay(10);
						}

						if(block_num == read_win.block_num) //是读cache的数据
						{
								memcpy(write_win[write_cache_num].buff, read_win.buff, SD_BLOCK_SIZE); //将数据拷贝到写缓存中
						}
						else
						{
								res = sd_read_block(block_num, write_win[write_cache_num].buff, 0); //将数据读出
								if(res)
								{
										return 0;
								}
						}
						write_win[write_cache_num].block_num  = block_num;
				}


				sd_printf("write block %d :%d  %d:%d\n\r", !write_cache_num, write_win[!write_cache_num].block_num, write_cache_num, write_win[write_cache_num].block_num);

				//将cache数据同步到sd卡,先读后写，写不需要等待，可提高效率
				if(write_win[!write_cache_num].block_num < 0xffffffff)
				{

						res = sd_write_block(write_win[!write_cache_num].block_num, write_win[!write_cache_num].buff, 0);
						if(res)
						{
								return 0;
						}

						if(write_win[!write_cache_num].block_num == read_win.block_num) //读cache与当前写的一致 还要更新读cache
						{
								memcpy(read_win.buff, write_win[!write_cache_num].buff, SD_BLOCK_SIZE); 
						}
				}
		}

		memcpy(write_win[write_cache_num].buff+offset, buff, buff_len);

		return buff_len;
}

//同步写cache的数据到sd卡
u8 sync_sd(void)
{
    u8 res;
    if(write_win[write_cache_num].block_num < 0xffffffff)
    {
        res = sd_write_block(write_win[write_cache_num].block_num, write_win[write_cache_num].buff, 0);
        if(res)
        {
             return res;
        }

        if(write_win[write_cache_num].block_num == read_win.block_num) //读cache与当前写的一致 还要更新读cache
        {
            memcpy(read_win.buff, write_win[write_cache_num].buff, SD_BLOCK_SIZE); 
        }
   }
    return 0;
}


u16 sd_read(u32 block_num, u16 offset, u8 *buff, u16 buff_len)
{
    u8 res;
    if(buff_len > (SD_BLOCK_SIZE - offset)) //超出块边界
    {
        buff_len = SD_BLOCK_SIZE - offset;
    }

    //先在写cache里判断是否有数据 ，写cahce里的数据时最新的
        if(block_num == write_win[write_cache_num].block_num) //写cache中有数据，不需要再去读
        {
             memcpy(buff, write_win[write_cache_num].buff+offset, buff_len); 
        }else if(block_num == write_win[!write_cache_num].block_num)
        {
             memcpy(buff, write_win[!write_cache_num].buff+offset, buff_len); 
        }
        else
        {
            if(read_win.block_num != block_num) //读cache里也没有数据
            {  
                if(buff_len == SD_BLOCK_SIZE)  //块读取直接返回
                {
                    res = sd_read_block(block_num, buff,1); 
                    if(res)
                    {
                        return 0;
                    }  
                    return SD_BLOCK_SIZE;
                }
                else
                {
                    read_win.block_num = block_num;
                    res = sd_read_block(block_num, read_win.buff,1); //将数据读入cache
                    if(res)
                    {
                        read_win.block_num = 0xffffffff;
                        return 0;
                    } 
                }
            }
            memcpy(buff, read_win.buff+offset, buff_len); 
        }       
    return buff_len;
}
#endif
u8 sd_pre_read(u32 block_num)
{
    u8 res;
    read_win.block_num = block_num;
    res = sd_read_block(block_num, read_win.buff,0); //将数据读入cache
    if(res)
    {
         read_win.block_num = 0xffffffff;
        return 1;
    }    
}

u8 sd_write_block(u32 block_num, u8 *buff,u8 wait_flag)
{
    u8 res;
		
    while(check_sdio_busy()) //等待接收完成
    {
        delay(10);
    }
    if(!wait_flag) //不等待写完成，需要保证缓存不会被释放
				memcpy(sd_write_buff, buff, SD_BLOCK_SIZE); 	

    res = load_cmd(&sdio_hd, WRITE_BLOCK, block_num);
    sdio_hd.buff = buff;
    sdio_hd.data_len = 512;
    sdio_hd.data_cnt = 0;
    sdio_hd.data_dir = 0;
    res = sdio_cmd_send(&sdio_hd);   

   if(wait_flag)
   {
        while(check_sdio_busy()) //等待写完成
        {
            delay(1);
        }
    }
    
    if(res)
        return 1;
   
    return 0;
}

inline u8 get_sd_busy()
{
		return check_sdio_busy();
}

u8 sd_read_block(u32 block_num, u8 * buff, u8 wait_flag)
{
    u8 res;
    while(check_sdio_busy()) //等待接收完成
    {
        delay(1);
    }
    
    res = load_cmd(&sdio_hd, READ_SINGLE_BLOCK, block_num);
    sdio_hd.buff = buff;
    sdio_hd.data_len = 512;
    sdio_hd.data_cnt = 0;
    sdio_hd.data_dir = 1;
    res = sdio_cmd_send(&sdio_hd);
  //  sd_printf("read res %d  sd sta:0x%x\n", res, sdio_hd.rsp[0]);
   if(wait_flag) {
        while(check_sdio_busy()){ //等待接收完成
            delay(1);
        }
    }
    
    if(res) {
        return 1;
    } else {
       //printf_buff(buff, 512);
    }
    return 0;
}



/*----------------------------------------------------------------
**	函数	：void _sd_init(void)
**	功能	：初始化SD卡
**	参数	：void
**	返回值：void
**	备注	：无
-------------------------------------------------------------------*/
u8 sd_init(void)
{
    u8 res;
    u16 i;
    u8 cnt = 100;

#if SD_MALLOC
    //分配cache空间
		u8 *buffer;
		#if WIN_CACHE
		buffer =(u8 *)malloc(SD_BLOCK_SIZE*4);
		if(buffer == NULL)
				return 2;

    write_win[0].buff				= buffer + SD_BLOCK_SIZE;
    write_win[0].block_num	= 0xffffffff;
    write_win[1].buff				= buffer + SD_BLOCK_SIZE*2;
    write_win[1].block_num	= 0xffffffff;
    read_win.buff						= buffer + SD_BLOCK_SIZE*3;
    read_win.block_num			= 0xffffffff;
		#else
		buffer = malloc(SD_BLOCK_SIZE);
		if(buffer == NULL)
				return 2;
		#endif

		sd_write_buff =(u8 *)malloc(SD_BLOCK_SIZE);
		if(sd_write_buff == NULL)
				return 2;
#else
		sd_write_buff				= &sd_write_buffer[0];
		write_win[0].buff		= &sd_win_cache[0][0];
		write_win[1].buff		= &sd_win_cache[1][0];
		read_wn.buff				= &sd_win_cache[2][0];
#endif
    
   res = open_sdio(&sdio_hd);  //打开sdio接口
   sd_printf("open sdio err:%d\n\r", res);
   if(res)
        return 1;
   
	 sd_send_cmd(GO_IDLE_STATE, 0);  //复位SD卡
	 sd_printf("GO_IDLE_STATE\n\r"); 
	 res =  sd_send_cmd(SDIO_SEND_IF_COND, 0x1aa);  
	 if(res)
			 return 3;

	 delay(10);
	 sd_printf("SD_BLOCK_SIZE\n\r"); 
	 while(cnt--) {   
			 sd_send_cmd(APP_CMD, 0);  //同步SD卡操作电压
			 sd_send_cmd(SD_APP_OP_COND, 0xc0100000);
		   delay(1000);
			 uprintf("rsp %x\n\r", sdio_hd.rsp[0]);
			 if(sdio_hd.rsp[0]&(1<<31)) {
					 break;
			 }
	 }
	 if(cnt == 0)
			 return 4;


	 sd_printf("ALL_SEND_CID\n\r"); 
	 res = sd_send_cmd(ALL_SEND_CID, 0); 
	 if(res)
			 return 4;

	 sd_printf("%s\n\r", __line__);
	 res =  sd_send_cmd(SEND_RELATIVE_ADDR, 0);  //获取SD卡的操作地址
	 if(res)
			 return 5;

	 sd_hd.rca = sdio_hd.rsp[0]&0xffff0000;

	 res =  sd_send_cmd(SEND_CSD, sd_hd.rca);    //获取指定SD卡的CSD
	 memcpy(&sd_hd.CSD, sdio_hd.rsp, 4*4);

	 res =  sd_send_cmd(SEND_CID, sd_hd.rca);    //获取指定SD卡的CID
	 memcpy(&sd_hd.CID, sdio_hd.rsp, 4*4);
	 sd_printf("CSD:0x%x-0x%x-0x%x-0x%x\n\r", sd_hd.CSD[0],sd_hd.CSD[1],sd_hd.CSD[2],sd_hd.CSD[3]);
	 analysis_CSD((void *)&sd_hd.CSD);//解析CSD
	 set_sdio_clk(49);  //设置时钟速度
	 sd_printf("5\n\r");

	 res =  sd_send_cmd(SELEC_CHIP, sd_hd.rca);  //选中SD卡
	 if(res)
			 return 6;

	 sd_printf("6\n\r");
	 sd_send_cmd(APP_CMD, sd_hd.rca);
	 res =  sd_send_cmd(SET_BUS_WIDTH, 0x02);  //4线模式
	 if(res)
			 return 6;

	 sd_printf("7\n\r");
	 if(sd_hd.block_size != SD_BLOCK_SIZE)
	 {
			 res =  sd_send_cmd(SET_BLOCKLEN, SD_BLOCK_SIZE);  //设置块大小为512B
			 if(res)
					 return 6;

	 }

	 return 0;
}
/*----------------------------------------------------------------
**	函数	：void _sd_init(void)
**	功能	：初始化SD卡
**	参数	：void
**	返回值：void
**	备注	：无
-------------------------------------------------------------------*/
void sd_exit()
{
#if SD_MALLOC
		free(sd_write_buff);
#endif
}

sd_handle *open_sd(void)
{
	
	if(sd_hd.init_flag == 0) {
		u8 res;
		res =  sd_init();  //初始化SD卡
		if(res)
			return 0;
		sd_hd.init_flag = 1;
	}
    sd_hd.io.init = sd_init;
    sd_hd.io.read = sd_read;
    sd_hd.io.write = sd_write;
    sd_hd.io.sync_sd = sync_sd;
    sd_hd.io.read_block = sd_read_block;
    sd_hd.io.write_block = sd_write_block;
    return &sd_hd;
}

