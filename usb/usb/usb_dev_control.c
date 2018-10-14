/***********************************************************************************************
 *
 *	日期: 2018/2/21  20:47
 *
 *	作者：liuxing
 *
 *  备注：usb 从机硬件控制层 
 *
 *
 * *******************************************************************************************/
#include <S3C2440.H>
#include "config.h"
#include "irq.h"
#include "usb_dev_control.h"

#define USB_DRV_DEBUG
#ifdef USB_DRV_DEBUG
#define usb_printf uprintf
#else
#define usb_printf(...) 
#endif

usb_event_callback event_callback = NULL;
static void usb_dev_isr();

void init_usb_dev_control(usb_event_callback event_cb)
{
			CLKCON |= BIT(7); //打开usb设备的时钟 
			CLKSLOW &= ~BIT(7);

			PWR_REG = 0x00;
			EP_INT_EN_REG = 0x1; //打开端点0中断使能
			USB_INT_EN_REG = BIT(2) | BIT(1) | BIT(0);

			INDEX_REG = 0; //配置端点0
			MAXP_REG = BIT(0); //默认端点0的大小为8个字节
		  register_irq(INT_USBD, 0, usb_dev_isr);
			event_callback = event_cb; 

			GPCCON &= ~(3 << 10);
			GPCCON |= BIT(10);
			GPCUP |= BIT(5);
			GPCDAT |= BIT(5);

		return NULL;
}

void close_usb_dev_control(void)
{
	CLKCON &= ~BIT(7); //关闭usb设备的时钟 
	GPCCON &= ~(3 << 10);
	GPCCON |= BIT(10);
	GPCUP &= ~BIT(5);
	GPCDAT &= ~BIT(5);

	event_callback = NULL;
  disable_irq(INT_USBD);
}
//事件回调函数
inline static void event_deal_cb(u16 event, u8 ep_index, u8 *buffer, u8 len)
{
		if(event_callback)
				event_callback(event, ep_index, buffer, len);
}
//获取输出端点fifo的数据大小
inline u8 get_fifo_cnt(u8 ep_index)
{
    u8 index_reg_back = INDEX_REG; //将原来选择的端点号保存
		u8 fifo_size = OUT_FIFO_CNT1_REG; //fifo中有多少数据
		INDEX_REG = index_reg_back;
		return fifo_size;
}


void fifo_flush(u8 ep_index)
{
    u8 index_reg_back = INDEX_REG; //将原来选择的端点号保存

		INDEX_REG = ep_index;
		if(ep_index == 0)
		{
				u8 data;
				while(OUT_FIFO_CNT1_REG)
				{
						data = EP0_FIFO;
				}
		}
		else
		{
				OUT_CSR1_REG |= EP_OUT_FLUSH_FIFO;
				IN_CSR1_REG |= EP_IN_FLUSH_FIFO;
		}

		INDEX_REG = index_reg_back;
}
/*----------------------------------------------------------------
**	function	：读取ep0 fifo
**	argument	： ep_index 端点号
								buff:读取数据的缓存
								size:缓存的大小
**	return	：读取的实际数据大小
**	note		：
-------------------------------------------------------------------*/
u8 read_ep_fifo(u8 ep_index, u8 * buff, u8 size)
{
    u8 index_reg_back = INDEX_REG; //将原来选择的端点号保存
		u8 fifo_size; 
		if(ep_index>4)
				return 0;

    INDEX_REG = ep_index;
		fifo_size = OUT_FIFO_CNT1_REG; //fifo中有多少数据
		if(size>fifo_size)
				size = fifo_size;   //不能读超出fifo的数据
		fifo_size = size;

		while(size--)
		{
				*buff++ = * (&EP0_FIFO + INDEX_REG * sizeof(volatile unsigned char *));
		}

		//清除OUT_PKT_RDY
		if(ep_index == 0)
				ep0_clear_out_rdy();
		else
				ep_clear_out_rdy();

		INDEX_REG = index_reg_back;
		return fifo_size;  //返回实际写入大小
}

u8 write_ep_fifo(u8 ep_index, u8 *buff, u8 size) 
{
    u8 index_reg_back = INDEX_REG;
		u8 fifo_size = size; 

		if(ep_index>4)
				return 0;
    INDEX_REG = ep_index;
		
		if(ep_index == 0)
				if(EP0_CSR & BIT(1))
						return 0;
		else
				if(IN_CSR1_REG & BIT(0))  //上一包数据没有发送完
						return 0;

		while(size--)
		{
				 *(&EP0_FIFO + INDEX_REG * sizeof(volatile unsigned char *)) = *buff++;
		}

//置位数据输入标志
		if(ep_index == 0)
				EP0_CSR |= BIT(1);
		else
				IN_CSR1_REG |= BIT(0);

    INDEX_REG = index_reg_back;
		return size;
}

static void  ep_isr_deal(u8 ep_index)
{
		if(ep_index)
		{
				if(usb_hw_ctrl(GET_EP_DIR, ep_index) == OUT_MODE)  //输出模式
				{
						if(OUT_CSR1_REG & EP_IN_SENT_STALL)
						{
								usb_printf("SENT_STALL\n\r");
								//	OUT_CSR1_REG &= ~SENT_STALL;
								usb_hw_ctrl(CLEAR_SENT_STALL, ep_index);
						}
						else if(OUT_CSR1_REG & EP_OUT_RDY)  //接收到主机的数据
						{
								u8 buffer[64];  //非0端点最大64个BYTE
								u8 buffer_size = read_ep_fifo(ep_index, buffer, 64);
								event_deal_cb(DATA_OUT_EVENT, ep_index, buffer, buffer_size);	
						}

				}
				else
				{

						if(IN_CSR1_REG & EP_IN_SENT_STALL)
						{
								usb_printf("SENT_STALL\n\r");
					//			IN_CSR1_REG &= ~BIT(5);
								usb_hw_ctrl(CLEAR_SENT_STALL, ep_index);
						}
						else if(IN_CSR1_REG & BIT(3))
						{
							//	usb_printf("FIFO_FLUSH\n\r");
								usb_hw_ctrl(EP_IN_FLUSH_FIFO, 0);	
						}
						else
								event_deal_cb(DATA_IN_EVENT, ep_index, 0, 0);	
						//		if(DATA_END == send_ep(ep_data[ep_index]))

				}
		}
		else //0号端点
		{
				if(EP0_CSR & EP0_OUT_RDY)
				{
						u8 pack_size; 
						u8 buffer[16]; //端点0最大16个BYTE 
						pack_size = read_ep_fifo(0, buffer, 16);
						event_deal_cb(DATA_OUT_EVENT, 0, buffer, pack_size);

				}
				else if(EP0_CSR & EP0_SETUP_END)
				{
						usb_printf("SETUP_END\n\r");
						usb_hw_ctrl(CLEAR_SETUP_END, 0);
						//usb_hw_ctrl(FLUSH_EP, 0);
					//	control_data_end();
				}
				else if(EP0_CSR & EP0_DATA_END)
				{
						usb_printf("DATA_END\n\r");
						usb_hw_ctrl(CLEAR_DATA_END, 0);
				}
				else
				{
						//	send_ep0();
						event_deal_cb(DATA_IN_EVENT, 0, 0, 0);
				}
		}
}
static void usb_dev_isr()
{
		u8 res = 0;

		if(USB_INT_REG & USB_RES)
		{
				usb_printf("\n\rusb rest\n\r");
				USB_INT_REG |= USB_RES;
				event_deal_cb(RES_EVENT, 0, 0, 0);
		}

		if(USB_INT_REG & USB_RESUME) 
		{
				usb_printf("RESUME\n\r");
				event_deal_cb(RESUME_EVENT, 0, 0, 0);
		}

		if(USB_INT_REG & USB_SUSPEND)
		{
				usb_printf("SUSPEND\n\r");
				event_deal_cb(SUSPEND_EVENT, 0, 0, 0);
		}

		if(EP_INT_REG)  //是端点中断
		{
				u8 ep_index;

				for(ep_index = 0; ep_index < 4; ep_index++)
				{
						if(EP_INT_REG & BIT(ep_index))
						{
								INDEX_REG = ep_index;
								EP_INT_REG |= BIT(ep_index);  //清除中断标志
								ep_isr_deal(ep_index);
						}
				}
		}
		ClearPending(INT_USBD);
}


/*----------------------------------------------------------------
**	function	：初始化ep端点
**	argument	ep_hd ep句柄
**	return	：错误号
**	note：
-------------------------------------------------------------------*/
u8 open_ep(u8 ep_index, u8 ep_size, u8 dir, u8 mode)
{
		if(ep_index > 4)
		{
				return 1;
		}

		EP_INT_EN_REG |= BIT(ep_index); //开端点中断
		INDEX_REG = ep_index;

		switch(ep_size)
		{
				case 8:
						MAXP_REG = EP_FIFO_8;
						break;
				case 16:
						MAXP_REG = EP_FIFO_16; 
						break;
				case 32:
						MAXP_REG = EP_FIFO_32;
						break;
				case 64:
						MAXP_REG = EP_FIFO_64;
						break;
				default:
						return 2;
		}

		if(ep_index == 0)  //ep0做控制传输 方向不需要配置
				return 0;

		if(dir == IN_MODE)
		{
				if(mode != EP_ATTR_BULK)
				{
						//	OUT_CSR2_REG = BIT(6);
						IN_CSR2_REG = IN_MODE;//BIT(5);
				}
				else
				{
						IN_CSR2_REG = IN_MODE;//BIT(5);
						//	OUT_CSR2_REG = 0;
				}
				IN_CSR1_REG = 0;
		}
		else if(dir == OUT_MODE)
		{
				if(mode != EP_ATTR_BULK)
				{
						IN_CSR2_REG = OUT_MODE;//BIT(6);
	//					OUT_CSR2_REG = OUT_MODE;// BIT(6);
				}
				else
				{
						IN_CSR2_REG = OUT_MODE;//0x00;
//						OUT_CSR2_REG = OUT_MODE; //0x00;
				}
		}
		else
				return 2;

		return 0;
}
void close_ep(u8 ep_index)
{
		if(ep_index > 4)
		{
				return;
		}

		EP_INT_EN_REG &= ~BIT(ep_index); //关端点中断
}

u32 usb_hw_ctrl(u8 CMD, u32 data)
{
    u8 index_reg_back = INDEX_REG;
		u8 res = 0;
		switch(CMD)
		{
				case CLEAR_SETUP_END:
						EP0_CSR |= BIT(7); 
						break;	
				case FLUSH_EP:
						fifo_flush(data);
						break;
				case CLEAR_DATA_END:
						EP0_CSR |= EP0_DATA_END;
						break;
				case CLEAR_SENT_STALL:
						INDEX_REG = data;
						IN_CSR1_REG |= EP_IN_SENT_STALL;
						OUT_CSR1_REG |= EP_OUT_SENT_STALL;
						break;
				case GET_EP_DIR:
						INDEX_REG = data;
						res = IN_CSR2_REG&IN_MODE;
						break;
				case GET_EP_SIZE:
						INDEX_REG = data;
						res = MAXP_REG;
						break;
				case GET_EP_SEND_STATUS:
						INDEX_REG = data;
						if(data)  //非0端点
								res = IN_CSR1_REG & EP_IN_RDY;
						else
								res = EP0_CSR & BIT(1);  

						break;
				default:
						break;
		}
		INDEX_REG = index_reg_back;
		return res;
}
