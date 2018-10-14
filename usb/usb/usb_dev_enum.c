#include "usb_dev_enum.h"
#include "SCSI.h"
#include "c_buff.h"
#include "task_manage.h"
#include "cmd.h"

_usb_drv_handle usb_hd;
#if USB_DEV_ENUM == USB_COM
const struct USB_DEVICE_DESCRIPTOR device_descriptor = //设备描述符
{
		sizeof(struct USB_DEVICE_DESCRIPTOR),//描述符大小
		DEVICE_TYPE,
		0x10,
		0x01,//USB版本为1.1 
		0x02,//设备类型码，通信设备类
		0x00,//子类型码
		0x00,//设备协议代码
		0x08,//端点0缓存大小8
		0x66,
		0x66,//厂商代码
		0x78,
		0x23,//产品编号
		0x00,
		0x01,//出厂序号
		0x00,//设备厂商字符串索引
		0x00,//描述产品字符串索引
		0x00,//描述设备序列号索引
		0x01,//可能的配置数量
};


const CONF_INT_ENDP_DESCRIPTOR config =
{
	{ //配置描述
		sizeof(struct USB_CONFIGURATION_DESCRIPTOR),
		CONFIGURATION_TYPE,
		sizeof(CONF_INT_ENDP_DESCRIPTOR) % 256,//32个字节
		sizeof(CONF_INT_ENDP_DESCRIPTOR) / 256,//sizeof(CONF_INT_ENDP_DESCRIPTOR)/256,
		0x02,//只有一个接口
		0x01,//配置编号
		0,
		0xa0,
		25 //最大电流50mA
	},

	//CDC interface_descritor //CDC 类接口描述符
	{
		sizeof(struct USB_INTERFACE_DESCRIPTOR),//描述符字节大小
		INTERFACE_TYPE,//接口描述符类型编号
		0x00,//接口编号为0
		0x00,//该接口描述符的备用编号
		0x00,//非0 端点数量为1，端点1 中断
		0x02,//所使用的类，Communication Interface Class
		0x02,//使用的子类，Abstract Control Model
		0x01,//使用的协议，Common AT commands
		0x00 //接口描述符字符串索引
	},

	//header_functional_descriptor
	{
		sizeof(HEADER_FUNCTIONAL_DESCRIPTOR),//Size of this descriptor in bytes
		CS_INTERFACE_DESCRIPTOR,//CS_INTERFACE descriptortype
		0x00,//Header functional descriptor subtype
		0x10, //CDC 版本号
		0x01
	},

	//call_management_functional_descriptor
	{
		sizeof(CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR),//Size of this descriptorin bytes
		CS_INTERFACE_DESCRIPTOR,//CS_INTERFACE descriptor type
		0x01,//Call Management functional descriptor subtype
		0x00,//D1=0: Device sends receives call management
		0x00 //Interface number of Data Class interface optionally used for call management(herenot use).
	},

	//abstract_ontrol_anagement_unctional_escriptor
	{
		sizeof(ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR),//Sizeof this descriptor in bytes
		CS_INTERFACE_DESCRIPTOR,//CS_INTERFACE descriptor type
		0x02,//Abstract Control Management functional descriptor subtype
		0x02,//D3=0: Device DO NOT supports the notification Network_Connection.

		//D2=0: Device DO NOT supports the request Send_Break
		//D1=0: Device supports the request combination of
		// Set_Line_Coding,Set_Control_Line_State,
		// Get_Line_Coding, and the notification Serial_State.
	},
	//union_functional_descriptor
	{
		sizeof(UNION_FUNCTIONAL_DESCRIPTOR),//Size of this descriptor in bytes
		CS_INTERFACE_DESCRIPTOR,//CS_INTERFACE descriptor type
		0x06,//Union functional descriptor SubType
		0x00,//MasterInterface，CDC 接口
		0x01 //SlaveInterface，数据接口
	},

	//endpoint1in_descriptor
	/*{
		sizeof(struct USB_ENDPOINT_DESCRIPTOR),//描述符字节大小
		ENDPOINT_TYPE,//描述符类型
		3 | EP_ADDR_IN,//1号输入端点
		EP_ATTR_INTERRUPT,
		0x20,
		0x00,
		0x80,
	},*/
	{ //接口描述符
		sizeof(struct USB_INTERFACE_DESCRIPTOR),//描述符字节大小
		INTERFACE_TYPE,//描述符类型
		0x01,//接口编号
		0x00,//备用编号
		0x02,//非0端点数
		0x0A,//使用的类
		0x00,//使用的子类
		0x00,//使用的协议
		0x00 //接口描述字符串索引
	},
	{
		{
			sizeof(struct USB_ENDPOINT_DESCRIPTOR),//描述符字节大小
			ENDPOINT_TYPE,//描述符类型
			2 | EP_ADDR_IN,//1号输入端点
			EP_ATTR_BULK,
			0x20,
			0x00,
			0x00,
		},
		{
			sizeof(struct USB_ENDPOINT_DESCRIPTOR),//描述符字节大小
			ENDPOINT_TYPE,//描述符类型
			1 | EP_ADDR_OUT,//1号输入端点
			EP_ATTR_BULK,
			0x20,
			0x00,
			0x00,
		}
	}
};


u8 sring3[] =
{
	0x0c,STRING_TYPE,'L',0x00,'X',0x00,'U',0x00,'S',0x00,'B',0x00
};


u8 sring1[] =
{
	0x04,STRING_TYPE,LANGID_US_L,LANGID_US_H
};


LINE_CODING LINE_COD =
{
	115200,
		0,
		0,
		8
};




/*---------------------------------------------------------------
		**	function	：USB 发送数据
		**	argument	：
		**	return	： 	错误号
		**	note		：
-------------------------------------------------------------------*/
u8 usb_write(void*buff,u16 len)
{
	return usb_hd.io.ep_write(buff,len,2);
}


/*---------------------------------------------------------------
		**	function	：USB数据接收回调
		**	argument	：read_data ：接收到的数据 size 数据大小
							ep_index 端点号
		**	return	：    
		**	note		：
-------------------------------------------------------------------*/
u8 usb_data_callback(u8*read_data,u16 size,u8 ep_index)
{
	if(!ep_index) {
		memcpy(&LINE_COD,read_data,7);
		uprintf("b %d\n\r",LINE_COD.dwDTERate);
	}
	else {
		usb_write(read_data,size);
		//usb_write((u8*)&"hello",6);
	}
	return 0;
}


/*---------------------------------------------------------------
		**	function	：类请求回调
		**	argument	：cmd 主机命令
		**	return	：    0 发送 data_end给主机
						 1 发送 空包给主机
						 2 接收下一包ep0的数据，从接收回调返回
						 other 不做其他操作
		**	note		：
-------------------------------------------------------------------*/
u8 class_callback(USB_SETUP_DATA*cmd)
{
	// uprintf("c 0x%x\n\r", cmd->bRequest);
	if(cmd->bmRequestType.dir==1) //命令方向 设备到主机
	{
		switch(cmd->bRequest)
		{
			case GET_LINE_CODING:
				write_ep((u8*)&LINE_COD,7,0);
				return 3;

				break;

			default:
				return 0;

				break;
		}
	}
	else {
		switch(cmd->bRequest)
		{
			case SET_CONTROL_LINE_STATE:
				return 0;

				break;

			case SET_LINE_CODING:
				return 2;

				break;

			default:
				return 1;
		}
	}
	return 0;
}


/*---------------------------------------------------------------
		**	function	：初始化虚拟串口
		**	argument	：void
		**	return	：    void
		**	note		：
-------------------------------------------------------------------*/
u8 open_usb_uart(void)
{
	descriptor_register(DEVICE_TYPE,(u8*)&device_descriptor,0); //注册设备描述符
	descriptor_register(CONFIGURATION_TYPE,(u8*)&config,0); //注册配置描述符
	descriptor_register(STRING_TYPE,(u8*)&sring3,1);
	descriptor_register(STRING_TYPE,(u8*)&sring3,2);
	descriptor_register(STRING_TYPE,(u8*)&sring1,0);
	open_usb(&usb_hd,usb_data_callback,class_callback);
	return 0;
}


void colse_usb_uart(void)
{
	set_get_ep0_data();
}


#endif

#if USB_DEV_ENUM == USB_DISK
const struct USB_DEVICE_DESCRIPTOR device_descriptor = //设备描述符
{
	sizeof(struct USB_DEVICE_DESCRIPTOR),//设备描述符的字节数大小
	DEVICE_TYPE,//设备描述符类型编号
	0x10,//USB 版本号
	0x01,
	0x00,//USB 分配的设备类代码
	0x00,//USB 分配的子类代码
	0x00,//USB 分配的设备协议代码
	0x08,//端点0 的最大包大小
	0x88,//厂商编号
	0x88,
	0x05,//产品编号
	0x00,
	0x00,//设备出厂编号
	0x01,
	0x01,//设备厂商字符串的索引
	0x02,//描述产品字符串的索引
	0x00,//描述设备序列号字符串的索引
	0x01 //可能的配置数量
};


const CON_INT_ENDP_DESCRIPTOR_STRUCT con_int_endp_descriptor =
{
	//configuration_descriptor //配置描述符
	{
		sizeof(struct USB_CONFIGURATION_DESCRIPTOR),//配置描述符的字节数大小
		CONFIGURATION_TYPE,//配置描述符类型编号
		sizeof(CON_INT_ENDP_DESCRIPTOR_STRUCT)%256,
		sizeof(CON_INT_ENDP_DESCRIPTOR_STRUCT)/256,
		0x01,//只包含一个接口
		0x03,//该配置的编号
		0x00,//iConfiguration 字段
		0xc0,//采用总线供电，不支持远程唤醒
		0x30 //从总线获取最大电流400mA
	},

	//interface_descritor //接口描述符
	{
		sizeof(struct USB_INTERFACE_DESCRIPTOR),//接口描述符的字节数大小
		INTERFACE_TYPE,//接口描述符类型编号
		0x00,//接口编号为0
		0x00,//该接口描述符的编号
		0x02,//非0 端点数量为2，只使用端点主端点输入和输出 
		0x08,//USB 大容量存储设备
		0x06,//使用的子类，为简化块命令
		0x50,//使用的协议，这里使用单批量传输协议
		0x02 //接口描述符字符串索引
	},

	//endpoint_descriptor[]
	{
		{
			sizeof(struct USB_ENDPOINT_DESCRIPTOR),//描述符字节大小
			ENDPOINT_TYPE,//描述符类型
			2 | EP_ADDR_IN,//1号输入端点
			EP_ATTR_BULK,
			0x40,
			0x00,
			0x00,
		},
		{
			sizeof(struct USB_ENDPOINT_DESCRIPTOR),//描述符字节大小
			ENDPOINT_TYPE,//描述符类型
			1 | EP_ADDR_OUT,//1号输入端点
			EP_ATTR_BULK,
			0x40,
			0x00,
			0x00,
		}
	}
};

  u8 sring3[] =
    {
        0x0c,STRING_TYPE,'L',0x00,'X',0x00,'U',0x00,'S',0x00,'B',0x00
    };
    
    
    u8 sring1[] =
    {
        0x04,STRING_TYPE,LANGID_US_L,LANGID_US_H
    };
    

static u8 csw_step = CBW_START; 
void set_csw_step(u8 step)
{
	csw_step = step;
}
u8 cbw_cmd_deal(CBW *cbw_cmd)
{
    u8 res;
	 CSW csw_res;
   // uprintf("0\n\r");
    
   //  uprintf("1\n\r");
    csw_res.dCSWTag = cbw_cmd->dCBWTag;
    csw_res.dCSWSignature = 0x53425355;
    if(cbw_cmd->dCBWDataTransgerLength == 0)  //不传输数据
    {
        csw_step = CSW_END;
    }
    else //传输数据
    {
     //   uprintf("'t'");
        csw_step = DATA_TRANSFER;
    }
   //  uprintf("2\n\r");
    res = scsi_cmd_deal(cbw_cmd->CBWCB, cbw_cmd->bCBWCBLength);
    csw_res.dCSWDataResidue =  0; //cbw_cmd->dCBWDataTransgerLength - res;
 //   if(res)
    {
			
		csw_res.bCSWStatus = res;
			     
    }

    if(res)
    {
         uprintf("cbw_err\n\r");
         while(get_ep_buff_free(2) < sizeof(CBW)) ;
         printf_buff((u8 *)cbw_cmd, sizeof(CBW));
    }
  //  else
    {
  //      csw_res.bCSWStatus = 0;
    }
    
    // uprintf("3\n\r");
    // 
   // delay(29);
	  if(csw_step == CSW_END)
		{
			 usb_write(&csw_res,13);
		//	  uprintf("2 %d\n\r",res);
			csw_step = CBW_START;
		}
    // uprintf("cbw_cmd \n\r");
    
  //  uprintf("CSW size:%d\n", sizeof(CSW));
    
    return 0;
}


extern c_buff_struct  *cbw_cmd_cbuf;

u8 usb_data_callback(u8 *read_data,u16 size,u8 ep_index)
{
   
    CBW cbw_cmd; 
    
    memcpy(&cbw_cmd, read_data, size);
//     uprintf("ep%d read size:%d\n\r", ep_index, size);
   // printf_buff(read_data, size);
    if(cbw_cmd.dCBWSignature == 0x43425355)
    {
        u8 res;
        res = write_cbuff(cbw_cmd_cbuf, (u8 *)&cbw_cmd, sizeof(CBW ));
        if(res == 0)
        {
            uprintf("cbuff over\n\r");
         }
         res =task_post_msg("usb_task", 2, CBW_CMD_ARRIVAL, sizeof(CBW));
         if(res)
        {
            uprintf("post over  %d\n\r", res);
         }
        uprintf("\n\rcb 0x%x ", cbw_cmd.CBWCB[0]);
        
    }
 //   cbw_cmd_deal(&cbw_cmd);
   /* switch(csw_step)
    {
        case CBW_START:
            if(size>sizeof(CBW))
            {
                size = sizeof(CBW);
            }
            
            
            csw_step = DATA_TRANSFER;
        break;
        
        default:
            csw_step = CBW_START;
        break;
    }*/
    
    
    return 0;
}
/*---------------------------------------------------------------
		**	function	：USB 发送数据
		**	argument	：
		**	return	： 	错误号
		**	note		：
-------------------------------------------------------------------*/
u8 usb_write(void *buff,u16 len)
{
	return usb_hd.io.ep_write(buff,len,2);
}

/*---------------------------------------------------------------
		**	function	：类请求回调
		**	argument	：cmd 主机命令
		**	return	：    0 发送 data_end给主机
						 1 发送 空包给主机
						 2 接收下一包ep0的数据，从接收回调返回
						 other 不做其他操作
		**	note		：
-------------------------------------------------------------------*/
const u8 store_num = 0;
u8 class_callback(USB_SETUP_DATA*cmd)
{
	 
	if(cmd->bmRequestType.dir==1) //命令方向 设备到主机
	{
		switch(cmd->bRequest)
		{
		    case 0xfe:
			write_ep((u8*)&store_num,1,0);
			default:
				return 1;

				break;
		}
	}
	else {
		switch(cmd->bRequest)
		{

			default:
				return 1;
		}
	}
	uprintf("c 0x%x\n\r", cmd->bRequest);
	 printf_buff((void *)cmd, 8);
	return 0;
}


/*---------------------------------------------------------------
		**	function	：初始化大容量设备
		**	argument	：void
		**	return	：    void
		**	note		：
-------------------------------------------------------------------*/

u8 open_usb_disk(void)
{
	descriptor_register(DEVICE_TYPE,(u8*)&device_descriptor,0); //注册设备描述符
	descriptor_register(CONFIGURATION_TYPE,(u8*)&con_int_endp_descriptor,0); //注册配置描述符
	descriptor_register(STRING_TYPE,(u8*)&sring3,1);
	descriptor_register(STRING_TYPE,(u8*)&sring3,2);
	descriptor_register(STRING_TYPE,(u8*)&sring1,0);
	open_usb(&usb_hd,usb_data_callback,class_callback);

	return 0;
}

#endif

#if USB_DEV_ENUM == USB_HID

const struct USB_DEVICE_DESCRIPTOR device_descriptor = //设备描述符
{
	sizeof(struct USB_DEVICE_DESCRIPTOR),//设备描述符的字节数大小
	DEVICE_TYPE,//设备描述符类型编号
	0x10,//USB 版本号
	0x01,
	0x00,//USB 分配的设备类代码
	0x00,//USB 分配的子类代码
	0x00,//USB 分配的设备协议代码
	0x08,//端点0 的最大包大小
	0x88,//厂商编号
	0x88,
	0x05,//产品编号
	0x00,
	0x00,//设备出厂编号
	0x01,
	0x01,//设备厂商字符串的索引
	0x02,//描述产品字符串的索引
	0x00,//描述设备序列号字符串的索引
	0x01 //可能的配置数量	
};

const unsigned char KeyBoardReportDescriptor[]=
{
		0x05, 0x01, // USAGE_PAGE (Generic Desktop)
		0x09, 0x06, // USAGE (Keyboard)
		0xa1, 0x01, // COLLECTION (Application)
		0x05, 0x07, // USAGE_PAGE (Keyboard)
		0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
		0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
		0x15, 0x00, // LOGICAL_MINIMUM (0)
		0x25, 0x01, // LOGICAL_MAXIMUM (1)
		0x75, 0x01, // REPORT_SIZE (1)
		0x95, 0x08, // REPORT_COUNT (8)
		0x81, 0x02, // INPUT (Data,Var,Abs)
		0x95, 0x01, // REPORT_COUNT (1)
		0x75, 0x08, // REPORT_SIZE (8)
		0x81, 0x03, // INPUT (Cnst,Var,Abs)
		0x95, 0x05, // REPORT_COUNT (5)
		0x75, 0x01, // REPORT_SIZE (1)
		0x05, 0x08, // USAGE_PAGE (LEDs)
		0x19, 0x01, // USAGE_MINIMUM (Num Lock)
		0x29, 0x05, // USAGE_MAXIMUM (Kana)
		0x91, 0x02, // OUTPUT (Data,Var,Abs)
		0x95, 0x01, // REPORT_COUNT (1)
		0x75, 0x03, // REPORT_SIZE (3)
		0x91, 0x03, // OUTPUT (Cnst,Var,Abs)
		0x95, 0x06, // REPORT_COUNT (6)
		0x75, 0x08, // REPORT_SIZE (8)
		0x15, 0x00, // LOGICAL_MINIMUM (0)
		0x25, 0xFF, // LOGICAL_MAXIMUM (255)
		0x05, 0x07, // USAGE_PAGE (Keyboard)
		0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
		0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
		0x81, 0x00, // INPUT (Data,Ary,Abs)
		0xc0 // END_COLLECTION
};
const u8 respond_interrupt_exmple[8] = {0x0, 0x00, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00};
const u8 respond_interrupt_exmple1[8] = {0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const CON_INT_ENDP_DESCRIPTOR_STRUCT con_int_endp_descriptor=
{
	//configuration_descriptor //配置描述符
	{
			sizeof(struct USB_CONFIGURATION_DESCRIPTOR), //配置描述符的字节数大小
			CONFIGURATION_TYPE, //配置描述符类型编号
			sizeof(CON_INT_ENDP_DESCRIPTOR_STRUCT)%256,
		  sizeof(CON_INT_ENDP_DESCRIPTOR_STRUCT)/256,
			0x01, //只包含一个接口
			0x01, //该配置的编号
			0x00, //iConfiguration 字段
			0xA0, //采用总线供电，支持远程唤醒
			0xC8 //从总线获取最大电流400mA
	},
		//interface_descritor //接口描述符
		{ 
				sizeof(struct USB_INTERFACE_DESCRIPTOR), //接口描述符的字节数 大小
				INTERFACE_TYPE, //接口描述符类型编号
				0x02, //接口编号为0
				0x00, //该接口描述符的编号
				ENDPOINT_NUMBER, //非0 端点数量为2，只使用 端点主端点输入和输出
				0x03, //人机接口设备（HID）类
				0x01, //使用的子类：支持BIOS 根启动Boot 的子类
				0x01, //使用的协议：键盘
				0x00 //接口描述符字符串索引
		},
		//hid_descriptor
		{
				sizeof(HID_DESCRIPTOR_STRUCT), //HID 描述符的字节数大小
				HID_DESCRIPTOR, //HID 描述符类型编号
				0x0110, //HID 类协议版本号，为1.1
				0x00, //固件的国家地区代号，0x21 为美国
				NUM_SUB_DESCRIPTORS, //下级描述符的数量
				{
						{
								REPORT_DESCRIPTOR, //下级描述符为报告描述符
								sizeof(KeyBoardReportDescriptor) //下级描述符的长度
						}
				},
		},
		//endpoint_descriptor[]
		{
				{ //主端点输入描述
						sizeof(struct USB_ENDPOINT_DESCRIPTOR),//描述符字节大小
						ENDPOINT_TYPE,//描述符类型
						1 | EP_ADDR_IN,//1号输入端点
						EP_ATTR_INTERRUPT,
//						EP_ATTR_BULK,
						0x08,
						0x00,
						0x0a,

				},
	/*			{ //主端点输出描述
						sizeof(struct USB_ENDPOINT_DESCRIPTOR),//描述符字节大小
						ENDPOINT_TYPE,//描述符类型
						2 | EP_ADDR_OUT,//2号输出端点
						EP_ATTR_INTERRUPT,
						0x40,
						0x00,
						0x0a,
				}*/
		}
};
 sring3[] =
{
     0x0c,STRING_TYPE,'L',0x00,'X',0x00,'U',0x00,'S',0x00,'B',0x00
};    
u8 sring1[] =
{
    0x04,STRING_TYPE,LANGID_US_L,LANGID_US_H
};

u8 usb_data_callback(u8 *read_data,u16 size,u8 ep_index)
{
   
	 usb_printf("d cb ");
	 printf_buff(read_data, size);
     return 0;
}


/*---------------------------------------------------------------
		**	function	：类请求回调
		**	argument	：cmd 主机命令
		**	return	：    0 发送 data_end给主机
						 1 发送 空包给主机
						 2 接收下一包ep0的数据，从接收回调返回
						 other 不做其他操作
		**	note		：
-------------------------------------------------------------------*/
u8 class_callback(USB_SETUP_DATA*cmd)
{
//	usb_printf("hid class cmd\n\r"); 
		usb_printf("c:0x%x\n", cmd->bRequest);
	switch(cmd->bRequest)
	{
		case SET_IDLE:
		usb_printf("idle\n\r");
		return 1;
		case GET_DESCRIPTOR:
			switch(cmd->bValueH)
			{
				case REPORT_DESCRIPTOR:
						write_ep((u8*)KeyBoardReportDescriptor,sizeof(KeyBoardReportDescriptor),0);
//					printf_buff((u8 *)cmd, sizeof(USB_SETUP_DATA));
						printf("r");
						return 3;
				break;
				default:
						printf_buff((u8 *)cmd, sizeof(USB_SETUP_DATA));
						break;
			}
		break;
		case SET_REPORT:
				write_ep(respond_interrupt_exmple1, sizeof(respond_interrupt_exmple1), 1);
				return 2;
		default:
			printf_buff((u8 *)cmd, sizeof(USB_SETUP_DATA));
		break;
	}
	return 0;
}


u8 interrupt_mode_deal(u8 ep_index, void *buff, u16 len)
{
		if(ep_index == 1)
		{
		}
		return 0;
}
void send_key()
{
	write_ep(respond_interrupt_exmple, sizeof(respond_interrupt_exmple), 1);
}
/*---------------------------------------------------------------
		**	function	：初始化HID设备
		**	argument	：void
		**	return	：    void
		**	note		：
-------------------------------------------------------------------*/
u8 open_usb_hid(void)
{
	descriptor_register(DEVICE_TYPE,(u8*)&device_descriptor,0); //注册设备描述符
	descriptor_register(CONFIGURATION_TYPE,(u8*)&con_int_endp_descriptor,0); //注册配置描述符
	descriptor_register(STRING_TYPE,(u8*)&sring3,1);
	descriptor_register(STRING_TYPE,(u8*)&sring3,2);
	descriptor_register(STRING_TYPE,(u8*)&sring1,0);
  interrupt_mode_callback_register(interrupt_mode_deal);
	return open_usb(&usb_hd,usb_data_callback,class_callback);
}
u8 key_do(char  **arg, int arg_num)
{
	send_key();
	return 0;
}

REGISTER_CMD(key, key_do,"发送HID按键");

#endif

