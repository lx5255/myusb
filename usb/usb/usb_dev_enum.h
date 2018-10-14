#ifndef _USB_DEV_
#define _USB_DEV_
#include "usb_drv.h"

#define USB_COM 1
#define USB_DISK 2
#define USB_HID	3

#define USB_DEV_ENUM 4//USB_DISK


#define ENDPOINT_NUM         2
//定义Abstract Control Model Requests 码
#define SEND_ENCAPSULATED_COMMAND 0x00
#define GET_ENCAPSULATED_RESPONSE 0x01
#define SET_COMM_FEATURE 0x02
#define GET_COMM_FEATURE 0x03
#define CLEAR_COMM_FEATURE 0x04
#define SET_LINE_CODING 0x20
#define GET_LINE_CODING 0x21
#define SET_CONTROL_LINE_STATE 0x22
#define SEND_BREAK 0x23
//定义Abstract Control Model Notifications 码
#define NETWORK_CONNECTION 0x00
#define RESPONSE_AVAILABLE 0x01
#define SERIAL_STATE 0x20l

#if USB_DEV_ENUM == USB_COM
typedef struct _CONF_INT_ENDP_DESCRIPTOR
{
	struct USB_CONFIGURATION_DESCRIPTOR config;  //配置描述符
	struct USB_INTERFACE_DESCRIPTOR interface;  //接口描述符
	HEADER_FUNCTIONAL_DESCRIPTOR header_functional_descriptor;
	CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR call_management_functional_descriptor;
	ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR abstract_ontrol_anagement_unctional_escriptor;
	UNION_FUNCTIONAL_DESCRIPTOR union_functional_descriptor;
	//struct USB_ENDPOINT_DESCRIPTOR endpoint3;
	struct USB_INTERFACE_DESCRIPTOR interface2;  //接口描述符
	struct USB_ENDPOINT_DESCRIPTOR endpoint[ENDPOINT_NUM];    //端点描述符
}CONF_INT_ENDP_DESCRIPTOR;

typedef struct _line_coding
{
	u32 dwDTERate; //波特率，多少bit/s
	u8 bCharFormat; //停止位。0 表示1 位停止位；1 表示1.5 位停止位；2 表示2 位停止位。
	u8 bParityType; //校验位。0 表示无校验位；1 表示奇校验；2 表示偶校验；3 表示标记；4表示空格
	u8 bDataBits; //数据位。可取5，6，7，8，16。
}LINE_CODING;
#endif

#if USB_DEV_ENUM == USB_DISK

//定义了USB Mass Storage 类的一些命令及数据结构
#define SUCCESS 0x00
#define FAIL 0x01
#define ERROR 0x02


//定义命令块封包(CBW)
typedef struct _CBW
{
unsigned long dCBWSignature; //CBW 标志
unsigned long dCBWTag; //命令块标签
unsigned long int dCBWDataTransgerLength; //数据包大小
unsigned char bmCBWFlags; //D7 位表示数据方向，0=
unsigned char bCBWLUN; //接收该命令的逻辑单元号LUN
unsigned char bCBWCBLength; //表示CBWCB 的长度
unsigned char CBWCB[16]; //子类命令
}
CBW, * pCBW;
//定义命令状态封包(CSW)
typedef struct _CSW
{
unsigned long dCSWSignature; //CSW 的标志
unsigned long dCSWTag; //命令状态标志
unsigned long dCSWDataResidue; //表示dCBWDataTransferLenhth 字段中主机希望的数据长度与实际发送的数据长度的差额
unsigned char bCSWStatus; //该命令执行的情况
}
CSW, * pCSW;

#define ENDPOINT_NUMBER         2

typedef struct _CON_INT_ENDP_DESCRIPTOR_STRUCT
{
    struct USB_CONFIGURATION_DESCRIPTOR configuration_descriptor;
    struct USB_INTERFACE_DESCRIPTOR interface_descritor;
    struct USB_ENDPOINT_DESCRIPTOR endpoint_descriptor[ENDPOINT_NUMBER];
}__attribute__((packed)) CON_INT_ENDP_DESCRIPTOR_STRUCT;

enum 
{
    CBW_START = 0x00,
    DATA_TRANSFER,
    CSW_END,
};

enum
{
    SIGNA_ERR = 0x01,
    
};

u8 usb_write(void *buff,u16 len);
u8 open_usb_disk(void);
void set_csw_step(u8 step);
u8 cbw_cmd_deal(CBW *cbw_cmd);

#endif

#if USB_DEV_ENUM == USB_HID
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

#endif
#endif


