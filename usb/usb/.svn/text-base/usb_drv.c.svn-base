/***********************************************************************************************
 *
 *	日期: 2018/2/21  20:47
 *
 *	作者：liuxing
 *
 *  备注：usb 从机通用枚举层 
 *
 *
 * *******************************************************************************************/
#include <S3C2440.H>
//#include "s3c2440.h"
#include "usb_drv.h"
#include "usb_dev_control.h"

static int * isr_handle_array = (void *) 0x33fb06b0;
static ep_data_handle * init_ep_data(u16 malloc_buff_size);
static void free_ep_data(ep_data_handle **ep_data);
static void usb_event_cb(u16 event, u8 ep_index, u8 *buffer, u8 buffer_len);
void init_usb();

ep_data_handle * ep_data[5];
struct USB_DEVICE_DESCRIPTOR * device_descriptor_point = NULL;
static u8 * configuration_descriptor_point = NULL;
static u8 * string_descriptor_point[5] ={NULL};
static u8 * interface_descriptor_point = NULL;
static u8(*data_arrive_callback) (u8 * buff, u16 size, u8 ep_index) = NULL;
static u8(*class_request_callback) (USB_SETUP_DATA * host_cmd);
/*----------------------------------------------------------------
**	function	：描述符注册
**	argument	：descriptor_type 描述符类型
                    descriptor 描述符
                    index: 字符串描述符偏移值
**	return	：
**	note：
-------------------------------------------------------------------*/
u8 descriptor_register(u8 descriptor_type, void * descriptor, u8 index)
{
		switch(descriptor_type)
		{
				case DEVICE_TYPE:
						device_descriptor_point					= descriptor;
						break;
				case CONFIGURATION_TYPE:
						configuration_descriptor_point	= descriptor;
						break;
				case STRING_TYPE:
						string_descriptor_point[index]	= descriptor;
						break;
				case INTERFACE_TYPE:
						interface_descriptor_point			= descriptor;
						break;
				case ENDPOINT_TYPE:
						break;
				default:
						return ARG_ERR;
		}
		return 0;
}

/*----------------------------------------------------------------
**	function	：打开USB
**	argument	：usb_hd   返回的USB句柄
                    receive_callback 接收回调函数
                    class_request_cb: 类请求处理回调函数
**	return	：错误号
**	note：
-------------------------------------------------------------------*/
u8 open_usb(_usb_drv_handle *usb_hd, u8(*receive_callback) (u8 *, u16, u8), u8(*class_request_cb) (USB_SETUP_DATA *))
{

		ep_data[0] = init_ep_data(128);

		if(ep_data[0] == 0) {
				return MEM_ERR;
		}
		ep_data[0]->buff_size = 128;

		usb_printf("dev size %d \n\r",sizeof(struct USB_DEVICE_DESCRIPTOR));	
		usb_printf("sizeof type %d", sizeof(request_type )); 
		if(usb_hd == NULL) {
				return HANDLE_ERR;
		}

		if(device_descriptor_point){ //从设备描述符解析出端点0的信息
				ep_data[0]->ep_size = device_descriptor_point->bMaxPacketSize0;
				ep_data[0]->index = 0;
		} else {
				return DESCRIPTOR_ERR;
		}

		data_arrive_callback = receive_callback;
		class_request_callback = class_request_cb;
		if(configuration_descriptor_point){ //解析出其他端点信息
				u8 * p_conf = configuration_descriptor_point;

				p_conf += *p_conf;
				while(p_conf < (configuration_descriptor_point + configuration_descriptor_point[3] *256 + configuration_descriptor_point[2])){ //是否到达末尾
						usb_printf("g 0x%x   0x%x",p_conf-configuration_descriptor_point , configuration_descriptor_point[3] *256 + configuration_descriptor_point[2]);
						if(p_conf[1] == ENDPOINT_TYPE){ //端点描述符
								u8 ep_indx = p_conf[2] &0x0f;

								usb_printf("get ep%d\n\r", ep_indx);
								if(p_conf[2] &0x80){ //输入端点
										ep_data[ep_indx] = init_ep_data(EP_BUFF_MAX);
								}else{ //输出端点 
										ep_data[ep_indx] = init_ep_data(64); //回调不需要太大缓存			
								}

								if(ep_data[ep_indx] == NULL) {
										return MEM_ERR;
								}

								ep_data[ep_indx]->direction = p_conf[2] &0x80;
								ep_data[ep_indx]->ep_attr = p_conf[3];
								ep_data[ep_indx]->ep_size = p_conf[4];
								ep_data[ep_indx]->index = ep_indx;

								if(p_conf[2] &0x80){ //输入端点
										ep_data[ep_indx]->buff_size = EP_BUFF_MAX;
								} else{//输出端点
										ep_data[ep_indx]->buff_size = 64;//回调不需要缓存
								}
						}
						p_conf += p_conf[0]; //下一个描述符
				}
		} else {
				return DESCRIPTOR_ERR;
		}

		usb_hd->io.data_arrive_callback = data_arrive_callback;
		usb_hd->io.ep_write = write_ep;
		usb_hd->descriptor.configuration_descriptor_point = interface_descriptor_point;
		usb_hd->descriptor.device_descriptor_point = device_descriptor_point;
		usb_hd->descriptor.interface_descriptor_point = interface_descriptor_point;
		usb_hd->descriptor.string_descriptor_point = string_descriptor_point;
		//init_usb();
		init_usb_dev_control(usb_event_cb);
		usb_printf("open usb sucess\n\r");
		return 0;
}

void close_usb()
{
		u8 ep_index;
		close_usb_dev_control();
		for(ep_index = 0; ep_index < 5; ep_index++)
				free_ep_data(&ep_data[ep_index]);
}
/*----------------------------------------------------------------
**	function	：为ep句柄分配空间
**	argument	：malloc_buff_size 分配空间大小
**	return	：错误号
**	note：
-------------------------------------------------------------------*/
static ep_data_handle * init_ep_data(u16 malloc_buff_size)
{
	ep_data_handle * new_ep_data = (ep_data_handle *)malloc(sizeof(ep_data_handle));

	if(new_ep_data == 0) {
		return 0;
	}

	if(malloc_buff_size) {
		new_ep_data->cbuff = new_cbuff(malloc_buff_size);
		if(new_ep_data->cbuff == 0) {
			return 0;
		}
	} else {
		new_ep_data->buff = 0;
	}

	new_ep_data->cnt = 0;
	new_ep_data->data_size = 0;
	new_ep_data->busy = 0; 
	new_ep_data->start_flag = 0;
	
	return new_ep_data;
}
/*----------------------------------------------------------------
**	function	：是否ep句柄
**	argument	ep_data 句柄的地址
**	return	：void
**	note：
-------------------------------------------------------------------*/
static void free_ep_data(ep_data_handle **ep_data)
{
    if(*ep_data == NULL) {
        return ;
    }

    if((*ep_data)->cbuff != NULL) {
				free_cbuff((*ep_data)->cbuff);
    }
   usb_printf("free ep %d", (*ep_data)->index);
   free(*ep_data);  
   *ep_data = NULL;
}

/*----------------------------------------------------------------
**	function	：清除句柄缓存的数据
**	argument	ep_index ep号
**	return	：void
**	note：
-------------------------------------------------------------------*/
static void clear_ep_data(u8 ep_index)
{
		if(ep_data[ep_index])
				flush_cbuff(ep_data[ep_index]->cbuff);
}
/*----------------------------------------------------------------
**	function	：发送端点的数据，
**	argument	：ep_handle：端点数据句柄
**	return	：错误号
**	note		：可在中断重加载
-------------------------------------------------------------------*/
static u8 send_ep(ep_data_handle * ep_handle)
{
		int pack_size = ep_handle->ep_size;
		u8 send_buffer[64];  //最大只有64byte

		pack_size = read_cbuff(ep_handle->cbuff, send_buffer, pack_size);  //从cbuff中获取一包要发送的数据 
		if(pack_size == 0){ //没有数据需要发送
				return DATA_END;
		}
		write_ep_fifo(ep_handle->index, send_buffer, pack_size);  //将数据写入端点fifo
		if(ep_handle->index == 0){ //控制端点
				if(get_cbuff_data_size(ep_handle->cbuff) == 0){ //最后的数据
						control_data_in_end();  //控制点，需要置位数据结束位
				}
		}
	return NO_ERR;
}

/*----------------------------------------------------------------
**	function	：向端点发送缓存中写入数据
**	argument	：
**	return	：错误号
**	note		：
-------------------------------------------------------------------*/
u8 write_ep(u8 * buff, u16 len, u8 ep_index)
{
    u8 res = NO_ERR;
		write_cbuff(ep_data[ep_index]->cbuff, buff, len);  //将数据存入cbuff缓存
		send_ep(ep_data[ep_index]);  
		return res;

}

u16 get_ep_buff_free(u8 ep_index)
{
		return get_cbuff_remain(ep_data[ep_index]->cbuff);
}

/*----------------------------------------------------------------
**	function	：获取描述符命令处理
**	argument	：host_cmd	命令包
**	return	：错误号
**	note		：
-------------------------------------------------------------------*/
static u8 get_descriptor_deal(USB_SETUP_DATA * host_cmd)
{
	switch(host_cmd->bValueH) {
			case DEVICE_TYPE:
					if(device_descriptor_point)
							write_ep((u8 *) device_descriptor_point, ((u8 *) device_descriptor_point)[0], 0);
					usb_printf("dev\n\r");
					break;

			case CONFIGURATION_TYPE:
					if(configuration_descriptor_point) {
							u16 conf_descriptor_size =((struct USB_CONFIGURATION_DESCRIPTOR *) configuration_descriptor_point)->wTotalLengthL +  ((struct USB_CONFIGURATION_DESCRIPTOR *) configuration_descriptor_point)->wTotalLengthH *256;

							if(host_cmd->bLength == 0xff) {
									conf_descriptor_size  = sizeof(struct USB_CONFIGURATION_DESCRIPTOR);
							}
							conf_descriptor_size = MIN(host_cmd->bLength , conf_descriptor_size);//host_cmd->bLength >conf_descriptor_size ? conf_descriptor_size: host_cmd->bLength;				             
							write_ep(configuration_descriptor_point, conf_descriptor_size, 0);
							//usb_printf("conf_descriptor_size1:%d\n\r", conf_descriptor_size);
							usb_printf("cnf\n\r");
					}
					break;
		case STRING_TYPE:
			if(string_descriptor_point[host_cmd->bValueL]) {
			    u8 len;
			    len =  MIN(host_cmd->bLength,*string_descriptor_point[host_cmd->bValueL]);//?4:host_cmd->bLength;
			   
				write_ep(string_descriptor_point[host_cmd->bValueL], len,0);
				 usb_printf("get str %d\n", host_cmd->bValueL);
				 
			}
			break;

		case INTERFACE_TYPE:
			if(interface_descriptor_point) {
				write_ep(interface_descriptor_point, interface_descriptor_point[0], 0);
			}
			break;

		case ENDPOINT_TYPE:
		//	control_data_in_end();
			break;

		default:
		//	control_data_in_end();
			break;
	}

	return 0;
}


static u8 pack_cnt = 0;

//设置下一次EP0的数据通过回调返回给上层
void set_get_ep0_data(void)
{
	pack_cnt++;
}

//获取ep0的数据
u8 get_ep0_data(u8 * data_buff, u8 len)
{
	if(pack_cnt) {
		if(data_arrive_callback)
			data_arrive_callback(data_buff, len, 0);

		pack_cnt--;
		return 1;
	} else {
		return 0;
	}
}

/*----------------------------------------------------------------
**	function	：主机命令处理函数
**	argument	：host_cmd	命令包
**	return	：错误号
**	note		：
-------------------------------------------------------------------*/
static u8 host_cmd_deal(USB_SETUP_DATA * host_cmd)
{
		usb_printf("cmd ");
		if(host_cmd->bmRequestType.recive_type == 1){ //到接口,类命令
				u8 res = 0;

				if(class_request_callback)
						res = class_request_callback(host_cmd);

				switch(res) {
						case 0:
								control_data_end();
								break;
						case 1:
								control_data_in_end();
								break;
						case 2:
								set_get_ep0_data();
								break;
						case 3:
								return 0;
						default:
								return UNKNOWN_CMD;
				}
		}
		else if(host_cmd->bmRequestType.recive_type == 2){  //到端点
				u8 index_reg_back = INDEX_REG;
				INDEX_REG = host_cmd->bRequest;
				usb_printf("OUT_FIFO_CNT1_REG:%d\n\r",OUT_FIFO_CNT1_REG);
				INDEX_REG = index_reg_back;
		} else {
				if(host_cmd->bmRequestType.dir == 1){ //命令方向 设备到主机
						switch(host_cmd->bRequest){ //标准请求命令
								case GET_DESCRIPTOR:
										get_descriptor_deal(host_cmd);
										break;
								default:
										control_data_in_end();
										usb_printf("unkonw cmd request 0x%x\n\r", host_cmd->bRequest);
										return UNKNOWN_CMD;
						}
				} else{ //主机到设备
						switch(host_cmd->bRequest) {
								case SET_ADDRESS:
										FUNC_ADDR_REG = BIT(7) | host_cmd->bValueL;
										control_data_end();
										clear_ep_data(0);
										usb_printf("new addr %d\n\r", get_cbuff_data_size(ep_data[0]->cbuff));
										break;
								case SET_CONFIGURATION:
										//	config_set_value = host_cmd->bValueL;
										clear_ep_data(0);
										control_data_in_end();
										usb_printf("set cfg %d\n\r", host_cmd->bValueL);
										break;
								default:
										control_data_in_end();
										return UNKNOWN_CMD;
										break;
						}
				}
		}
		return NO_ERR;
}
/*----------------------------------------------------------------
**	function	：主机命令处理函数
**	argument	：host_cmd	命令包
**	return	：错误号
**	note		：
-------------------------------------------------------------------*/
static void usb_event_cb(u16 event, u8 ep_index, u8 *buffer, u8 buffer_len)
{
		u8 res = 0;
		switch(event)	{
				case RES_EVENT:
						{
								u8 ep;
								for(ep = 1; ep <5; ep++) {
										if(ep_data[ep]) {
												open_ep(ep, ep_data[ep]->ep_size, ep_data[ep]->direction, ep_data[ep]->ep_attr);
										}
										clear_ep_data(ep);
								}
						}
						break;
				case DATA_OUT_EVENT:
						if(ep_index) { //非控制端点 之前传数据到上层
								if(data_arrive_callback)
										data_arrive_callback(buffer, buffer_len, ep_index);
						}else {		    
								if(get_ep0_data((u8 *)buffer, buffer_len)) {
										control_data_in_end();	
										return 0;
								}

								res = host_cmd_deal(buffer);
								if(res == UNKNOWN_CMD) {
										printf_buff((u8 *)buffer, buffer);
								}
						}
						break;
				case DATA_IN_EVENT:
						send_ep(ep_data[ep_index]);
						break;
				default:
						break;
		}
}
