#include "config.h"
#include "s3c2440_sdio.h"
#include "s3c2440_uart.h"
#include "S3C2440.H"

static int * isr_handle_array = (void *) 0x33fb06b0;
static  u8 sdio_isr();
static sdio_handle *sdio_hd = 0;
volatile u8 step = 0;  //0没有操作 1命令发出 2数据传输中 

/*----------------------------------------------------------------
**	函数	：
**	功能	：初始化SDIO模块
**	参数	：NULL
**	返回值：无
**	备注	：无
-------------------------------------------------------------------*/
void sdio_init(void)
{
		u32 i;

		SDIO->CON = (0x01<<8);
		CLCKCON |= 0x01<<9;
		SDIO->PRE = 500;  //SDIO时钟设为500K

		SDIO->BSIZE = 512;
		SDIO->DATCON = (1<<20)|(1<<19)|(1<<18);

		GPEUP &=  ~(0x1f<<6);
		GPECON &= ~((0x03<<10)|(0x03<<12)|(0x03<<14)|(0x03<<16)|(0x03<<18)|(0x03<<20));
		GPECON |= (0x02<<10)|(0x02<<12)|(0x02<<14)|(0x02<<16)|(0x02<<18)|(0x02<<20);
		SDIO->CON = (0x01<<2)|0x01; //设置读取等待 打开时钟|(0x3<<4)
	//	isr_handle_array[21] = (int)sdio_isr;
		register_irq(INT_SDI, 0, sdio_isr);  //打开sdio中断
	//	INTMSK &= ~(1 << 21);
		delay(1000);
}

void sdio_exit()
{
		disable_irq(INT_SDI);  //关闭sdio中断
		SDIO->CON &= ~0x01; //关sdio模块
		CLCKCON &= ~BIT(9); //关sdio时钟
}

inline u8 check_sdio_busy(void)
{
    return step;
}
inline void set_sdio_clk(u8 pre)
{
    SDIO->PRE = pre;
}
u8 open_sdio(sdio_handle *hd)
{
    if(hd == 0)
    {
        return 1;
    }
    sdio_hd = hd;
    sdio_hd->buff = 0;
    sdio_hd->data_cnt = 0;
    sdio_hd->data_len = 0;
    sdio_hd->sta_flag = 0;
    sdio_hd->cmd.cmd = 0;
    sdio_hd->cmd.withdata = 0;
    step = 0;
    sdio_init();
    return 0;
}

u8 sdio_send_data(u8 start)
{
		//u8 data_cnt = 32;
		if(step == 0)
				return 1;

		if(sdio_hd == 0)
				return 2;

		if(sdio_hd->buff == 0)
				return 2;

		SDIO->TIMER = 0x2ffff;
		SDIO->BSIZE = BLOCK_SIZE;
		SDIO->DATCON = DATASIZE_U8|BLKMODE_BLOCK|WIDEBUS_4BIT|SEND_DATA_MODE|0X01;

		while(SDIO->FSTA&TF_DET)
		{
				if(sdio_hd->data_cnt >= sdio_hd->data_len)
				{
						break;
				}
				SDIDATB = sdio_hd->buff[sdio_hd->data_cnt++];
		}

		//   sd_printf("SDIO->DATCNT %d data_cnt:%d\n\r", SDIO->DATCNT, sdio_hd->data_cnt);
		if((sdio_hd->data_cnt >= BLOCK_SIZE))
		{
				// step = 0;
				//  SDIO->DATCON = 0;
		}

		if(start)
				SDIO->DATCON |= DATA_START; //开始传输

		return 0;

}

u8 sdio_recive_data(u8 start)
{
 
    if(step == 0)
        return 1;

    if(sdio_hd == 0)
        return 2;
    
    if(sdio_hd->buff == 0)
        return 2;
  
    SDIO->TIMER = 0x2ffff;
    SDIO->BSIZE = BLOCK_SIZE;
    SDIO->DATCON = DATASIZE_U8|BLKMODE_BLOCK|WIDEBUS_4BIT|RECIVE_DATA_MODE|0x01;
 
    while(SDIO->FSTA&RF_DET)
    {
       // data_cnt--;
        if(sdio_hd->data_cnt >= sdio_hd->data_len)
            break;

        sdio_hd->buff[sdio_hd->data_cnt++] = SDIDATB;
    }

    if(start)
        SDIO->DATCON |= DATA_START; //开始传输
   
    return 0;
 }
/*----------------------------------------------------------------
**	函数	：u8 sdio_send_cmd(u8 cmd, u32 arg, u32 *response)
**	功能	：发送一个命令
**	参数	：cmd:命令码	arg:命令参数	response：响应值缓存区
**	返回值：错误号
**	备注	：无
-------------------------------------------------------------------*/
u8 sdio_cmd_send(sdio_handle *hd)
{
		u8  res = 0;
		u32 msk = 0;
		if(hd == 0)
		{
				return 1;
		}
		if(check_sdio_busy()) //检查忙碌
		{
				return 2;
		}
		//	sd_printf("s1\n\r");
		sdio_hd = hd;
		SDIO->CMDSTA |= 0x1f00; //清除命令状态
		SDIO->CMDARG = sdio_hd->cmd.arg;
		SDIO->CMDCON = (sdio_hd->cmd.cmd&0x3f)|(1<<6);//带起始位的命令

		if(sdio_hd->cmd.abortcmd)  //终止命令
		{
				SDIO->CMDCON |=1<<12;
		}

		if(sdio_hd->cmd.withdata)
		{
				SDIO->CMDCON |=1<<11;
				msk |= RECIBE_CRC_ERR_INT|DATATIMEOUT_INT|DATAFIN_INT|TFHALF_INT|REHAFL_INT;
		}

		if(sdio_hd->cmd.rsp_type)  
		{
				sdio_hd->rsp[0] = 0;
				sdio_hd->rsp[1] = 0;
				sdio_hd->rsp[2] = 0;
				sdio_hd->rsp[3] = 0;
				SDIO->CMDCON |=1<<9;
				msk |= RSPCRC_INT|RSPEND_INT;
				if(sdio_hd->cmd.rsp_type == 2)
				{
						SDIO->CMDCON |=1<<10;

				} 
		}
		else
		{
				msk |= CMD_SEND_INT;
		}

		msk |= CMDTIMEOUT_INT|CRCSTA_INT;

		step = 1;
		sdio_hd->sta_flag  = 0;
		SDIO->CMDCON |= 0x01<<8; //启动命令传输
		SDIO->INT_MSK = msk;   

		if(sdio_hd->cmd.withdata)
		{     
				sdio_hd->data_cnt = 0;
				SDIO->DATSTA = 0x0fff;  //清除数据状态
				SDIO->FSTA  =(0x1<<16);     //清除FIFO

				if(sdio_hd->data_dir)
				{
						sdio_recive_data(1);
						//   sd_printf("recive start; res \n\r", res);
				}
				else
				{
						//   sd_printf("send start;\n\r");
						sdio_send_data(1);
				}

		}
		while(step==1)
		{
				//  		sd_printf(".");
				delay(10);
		}

		//	sd_printf("s3\n\r");    
		if(sdio_hd->sta_flag)
		{
				return 3;
		}
		return 0;

}
static u8 restat_cnt = 0;
u8 re_start_cmd()
{
   
    if(restat_cnt++ == 5)
    {
        restat_cnt = 0;
        return 1;
    }

	SDIO->CMDCON |= 0x01<<8; //启动命令传输

   step = 1;
   sdio_hd->sta_flag  = 0;
   if(sdio_hd->cmd.withdata)
   {     
       sdio_hd->data_cnt = 0;
       SDIO->DATSTA = 0x0fff;  //清除数据状态
       SDIO->FSTA  =(0x1<<16);     //清除FIFO

       if(sdio_hd->data_dir)
       {
            sdio_recive_data(1);
       }
       else
       {
            sdio_send_data(1);
       }
       
   }
    return 0;
}
u8  sdio_isr(void)
{
		//  sd_printf("sd isr\n\r");
		if(sdio_hd)
		{
				//对命令状态寄存器的处理
				if(SDIO->CMDSTA & RSPCRC_ERR)  //应答CRC出错
				{
						sd_printf("cmd crc\n\r");
		//				if(re_start_cmd() == 0)
		//				{
		//						return 1;
	//					}
						sdio_hd->sta_flag |= CRC_ERR;
						step = 0;
						SDIO->CMDSTA |= RSPCRC_ERR; 
						sdio_hd->rsp[0] = SDIO->RSP0;    
						return 1;
				}

				if(SDIO->CMDSTA & CMDTOUT)  //命令超时
				{      
						sd_printf("cmd time out\n\r");
						if(re_start_cmd() == 0)
						{
								return 1;
						}
						sdio_hd->sta_flag |= TOUT;
						step = 0;
						SDIO->CMDSTA |=  CMDTOUT;
						return 1;
				}
				if(sdio_hd->cmd.rsp_type == 0)
				{
						if(SDIO->CMDSTA & CMD_SEND_END)  //命令发送完成
						{
								sd_printf("cmd send\n\r");
								SDIO->CMDSTA |= CMD_SEND_END;
								step = 0;
						}
				}
				else
				{
						if(SDIO->CMDSTA & RSPFIN)  //接收响应完成
						{
								sd_printf("cmd rsp\n\r");
								if(sdio_hd->cmd.withdata)  //带数据 传输没有结束
								{
										step = 2; //数据传输阶段
								}
								else
								{
										step = 0;
								}

								if(sdio_hd->cmd.rsp_type == 2)  //长响应
								{
										sd_printf("rsp_type %d\n\r", step);
										sdio_hd->rsp[0] = SDIO->RSP3;
										sdio_hd->rsp[1] = SDIO->RSP2;
										sdio_hd->rsp[2] = SDIO->RSP1;
										sdio_hd->rsp[3] = SDIO->RSP0;
								}
								else  //短响应
								{
										sdio_hd->rsp[0] = SDIO->RSP0;
								}
								//	        sd_printf("rsp:0x%x\n", sdio_hd->rsp[0]);
								SDIO->CMDSTA |= RSPFIN;
						}
				}
				/*
					 if((SDIO->CMDSTA & 0x3f) != sdio_hd->cmd.cmd)  //返回的命令不一致
					 {
					 sd_printf("CMD_MATCH\n\r");
					 if(re_start_cmd() == 0)
					 {
					 return 1;
					 }
					 step = 0;
					 sdio_hd->sta_flag |= CMD_MATCH;
					 }
					 */
				//对数据状态寄存器的处理
				if(SDIO->DATSTA & BUSYTFIN_STA)
				{
						sd_printf("busy %d\n\r", SDIO->DATSTA & NOBUSY_STA);
						if((SDIO->DATSTA & NOBUSY_STA)==0)  
						{

								sdio_hd->sta_flag |= SD_BUSY;

						}
						else
						{
								SDIO->DATSTA |= NOBUSY_STA;
						}
						step = 0;
						SDIO->DATSTA |= BUSYTFIN_STA;
				}

				if(sdio_hd->cmd.withdata)
				{
						if(SDIO->DATSTA & CRCERR_STA) //发送CRC校验失败
						{
								sd_printf("CRCERR_STA\n\r");
								if(re_start_cmd() == 0)
								{
										return 1;
								}
								sdio_hd->sta_flag |= CRC_ERR;
								step = 0;
								SDIO->DATSTA |= CRCERR_STA;
								return 1;
						}
						if(SDIO->DATSTA & DATACRC_STA)  //接收CRC校验失败
						{
								sd_printf("DATACRC_STA\n\r");
								if(re_start_cmd() == 0)
								{
										return 1;
								}
								sdio_hd->sta_flag |= CRC_ERR;
								step = 0;
								SDIO->DATSTA |= DATACRC_STA;
								return 1;
						}

						if(SDIO->DATSTA & DATATIMEOUT_STA) //数据传输超时
						{    
								SDIO->DATSTA |=  DATATIMEOUT_STA; 
								sd_printf("DATATIMEOUT_STA\n\r");
								if(step)
								{
										if(re_start_cmd() == 0)
										{
												return 1;
										}
										sdio_hd->sta_flag |= TOUT;
										step = 0;
								}

								return 1;
						}

						if(SDIO->DATSTA & DATFIN_STA) //数据传输结束
						{     
								//  sdio_hd->sta_flag |= TOUT;
								step = 0;
								SDIO->DATSTA |=  DATFIN_STA;
								SDIO->DATCON = 0;
								//     sd_printf("data end\n\r");
								return 1;
						}
						//fifo状态寄存器处理
						if(sdio_hd->data_dir == 1)  //数据接收模式
						{

								if(SDIO->FSTA & RF_HAFL)  //有一半数据
								{
										//   sd_printf("RF_HAFL %d  %d\n\r", SDIO->FSTA&0x7f,  SDIO->DATCNT);
										sdio_recive_data(0);
								}
								else if(SDIO->FSTA & RF_LAST)
								{
										//  SDIO->FSTA |= RF_LAST;

										sdio_recive_data(0);
										//     sd_printf("RF_end  %d\n\r", sdio_hd->data_cnt);
								}
						}
						else //数据发送模式
						{
								if(SDIO->FSTA & TF_HALF)  //有一半数据
								{
										sd_printf("TF_HALF 0x%x\n\r", SDIO->DATSTA);
										sdio_send_data(0);
								}
						}

				}
				else
				{
						SDIO->DATSTA = 0x0fff;  //清除数据状态
				}
		}
		else
		{
				SDIO->CMDSTA |= 0x1f00; //清除命令状态
				SDIO->DATSTA = 0x0fff;  //清除数据状态
		}
		SRCPND |= BIT(25);
		INTPND |= BIT(25);
		return 0;
}

