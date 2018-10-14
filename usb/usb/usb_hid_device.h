#ifndef _USB_HID_DEV_
#define _USB_HID_DEV_
#include "config.h"
#include "hid_device_api.h"
#include "usb_drv.h"
//定义USB HID 特定命令请求
#define GET_REPORT 0x01 //获取报告
#define GET_IDLE 0x02 //获取空闲状态
#define GET_PROTOCOL 0x03 //获取协议
#define SET_REPORT 0x09 //设置报告
#define SET_IDLE 0x0A //设置空闲
#define SET_PROTOCOL 0x0B //设置协议

#define HID_DESCRIPTOR 0x21 //HID 描述符
#define REPORT_DESCRIPTOR 0x22 //报告描述符
#define PHYSICAL_DESCRIPTOR 0x23 //物理描述符

#define ENDPOINT_NUMBER        1 
//HID 描述符中的下级描述符号
typedef struct _HID_SUB_DESCRIPTOR_STRUCT
{
		u8 bDescriptorType; //下级描述符的类型编号
		u16 wDescriptorLength; //下级描述符的长度
}__attribute__((packed)) HID_SUB_DESCRIPTOR_STRUCT,*pHID_SUB_DESCRIPTOR_STRUCT;
#define NUM_SUB_DESCRIPTORS 1
//定义HID 描述符结构
typedef struct _HID_DESCRIPTOR_STRUCT
{
		u8 bLength;
		u8 bDescriptorType;
		u16 bcdHID;
		u8 bCountryCode;
		u8 bNumDescriptors;
		HID_SUB_DESCRIPTOR_STRUCT HidSubDescriptors[NUM_SUB_DESCRIPTORS];
}__attribute__((packed)) HID_DESCRIPTOR_STRUCT,*pHID_DESCRIPTOR_STRUCT;
//定义配置，接口，端点等描述符结构体，因为它们是随配置描述符一起返回的
typedef struct _CON_INT_ENDP_DESCRIPTOR_STRUCT
{
		struct USB_CONFIGURATION_DESCRIPTOR configuration_descriptor;
		struct USB_INTERFACE_DESCRIPTOR interface_descriptor;
		HID_DESCRIPTOR_STRUCT hid_descriptor;
		struct USB_ENDPOINT_DESCRIPTOR endpoint_descriptor[ENDPOINT_NUMBER];
}__attribute__((packed)) CON_INT_ENDP_DESCRIPTOR_STRUCT;

void register_usb_hid_callback(read_report_map_cb  read_report_map_callback,output_report_cb  output_report_callback);
void usb_hid_input_report_send(u8 report_id, u8 *buffer, u16 buffer_len);
u8 open_usb_hid(void);
void close_usb_hid();
#endif
