/***********************************************************************************************
 *
 *	日期: 2018/2/21  20:47
 *
 *	作者：liuxing
 *
 *  备注：usb hid设备驱动
 *
 *
 * *******************************************************************************************/
#include "usb_hid_device.h"

static _usb_drv_handle usb_hd;
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

CON_INT_ENDP_DESCRIPTOR_STRUCT con_int_endp_descriptor=
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
								0,//sizeof(KeyBoardReportDescriptor) //下级描述符的长度
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

				}/*,
				{ //主端点输出描述
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
const u8 respond_null[8] = {0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static read_report_map_cb usb_read_hid_map = NULL;
static output_report_cb   usb_hid_output_report_cb = NULL;

void register_usb_hid_callback(read_report_map_cb  read_report_map_callback,output_report_cb  output_report_callback)
{
		usb_read_hid_map = read_report_map_callback;
		usb_hid_output_report_cb  = output_report_callback;
}

u8 usb_data_callback(u8 *read_data,u16 size,u8 ep_index)
{
   
	 usb_printf("d cb ");
	 printf_buff(read_data, size);
	 if(usb_hid_output_report_cb)
				usb_hid_output_report_cb(0, read_data, size);
     return 0;
}

void usb_hid_input_report_send(u8 report_id, u8 *buffer, u16 buffer_len)
{
		u8 report[64];
		if(report_id != 0xff){
				buffer_len = buffer_len > 64?64:buffer_len;
				write_ep(buffer,buffer_len, 1); //发送报告描述符 

		}else{
				report[0] = report_id;
				buffer_len = buffer_len > 63?63:buffer_len;
				memcpy(&report[1], buffer, buffer_len);
				write_ep(report,buffer_len+1, 1); //发送报告描述符 
		}
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
//						write_ep((u8*)KeyBoardReportDescriptor,sizeof(KeyBoardReportDescriptor),0);
//					printf_buff((u8 *)cmd, sizeof(USB_SETUP_DATA));
						if(usb_read_hid_map)
						{
								u8 map_buff[128];
								u16 offset = 0;
								u16 pack_size;
								u16 map_size = get_report_map_size();  //获取报告描述符大小
								for(; offset <  map_size; offset += pack_size)
								{
									//	usb_printf("pack size %d offset %d map_size %d\n\r", pack_size, offset, map_size);
										pack_size = map_size - offset;
										if(pack_size > 128)
												pack_size = 128;

								//		usb_printf("map_size %d", map_size);
										 pack_size = usb_read_hid_map(offset, map_buff, pack_size);
//										uprintf("pack size %d offset %d\n\r", pack_size, offset);
										write_ep(map_buff,pack_size, 0); //发送报告描述符 
								}
						}
						uprintf("r");
						return 3;
				break;
				default:
						printf_buff((u8 *)cmd, sizeof(USB_SETUP_DATA));
						break;
			}
		break;
		case SET_REPORT:
			//	write_ep(respond_null, sizeof(respond_null), 1);
				return 2;
		default:
			printf_buff((u8 *)cmd, sizeof(USB_SETUP_DATA));
		break;
	}
	return 0;
}


/*---------------------------------------------------------------
	**	function	：初始化HID设备
	**	argument	：void
	**	return	：  void
	**	note		：
-------------------------------------------------------------------*/
u8 open_usb_hid(void)
{
	descriptor_register(DEVICE_TYPE,(u8*)&device_descriptor,0); //注册设备描述符
  con_int_endp_descriptor.hid_descriptor.HidSubDescriptors[0].wDescriptorLength = get_report_map_size(); //获取hid报告描述符大小
	descriptor_register(CONFIGURATION_TYPE,(u8*)&con_int_endp_descriptor,0); //注册配置描述符
	descriptor_register(STRING_TYPE,(u8*)&sring3,1);
	descriptor_register(STRING_TYPE,(u8*)&sring3,2);
	descriptor_register(STRING_TYPE,(u8*)&sring1,0);
 // interrupt_mode_callback_register(interrupt_mode_deal);
	open_usb(&usb_hd,usb_data_callback,class_callback);
	return 0;
}

void close_usb_hid()
{
		close_usb();
}

