#ifndef _USB_DRV_
#define _USB_DRV_
#include "config.h"
#include "c_buff.h"

#define USB_DRV_DEBUG
#ifdef USB_DRV_DEBUG
#define usb_printf uprintf
#else
#define usb_printf(...)   
#endif
// Standard bmRequestTyje (Direction) 
#define HOST_TO_DEVICE              (0x00)
#define DEVICE_TO_HOST              (0x80)    

// Standard bmRequestType (Type) 
#define STANDARD_TYPE               (0x00)
#define CLASS_TYPE                  (0x20)
#define VENDOR_TYPE                 (0x40)
#define RESERVED_TYPE               (0x60)

// Standard bmRequestType (Recipient) 
#define DEVICE_RECIPIENT            (0)
#define INTERFACE_RECIPIENT         (1)
#define ENDPOINT_RECIPIENT          (2)
#define OTHER_RECIPIENT             (3)

// Feature Selectors 
#define DEVICE_REMOTE_WAKEUP        (1)
#define EP_STALL                    (0)

// Standard Request Codes 
#define GET_STATUS                  (0)
#define CLEAR_FEATURE               (1)
#define SET_FEATURE                 (3)
#define SET_ADDRESS                 (5)
#define GET_DESCRIPTOR              (6)
#define SET_DESCRIPTOR              (7)
#define GET_CONFIGURATION           (8)
#define SET_CONFIGURATION           (9)
#define GET_INTERFACE               (10)
#define SET_INTERFACE               (11)
#define SYNCH_FRAME                 (12)

// Class-specific Request Codes 
#define GET_DEVICE_ID               (0)
#define GET_PORT_STATUS             (1)
#define SOFT_RESET                  (2)

// Descriptor Types
#define DEVICE_TYPE                 (1)
#define CONFIGURATION_TYPE          (2)
#define STRING_TYPE                 (3)
#define INTERFACE_TYPE              (4)
#define ENDPOINT_TYPE               (5)
//类特殊接口描述符类型
#define CS_INTERFACE_DESCRIPTOR    (0x24)
//configuration descriptor: bm Attributes 
#define CONF_ATTR_DEFAULT	    (0x80) //Spec 1.0 it was BUSPOWERED bit.
#define CONF_ATTR_REMOTE_WAKEUP     (0x20)
#define CONF_ATTR_SELFPOWERED       (0x40)

//endpoint descriptor
#define EP_ADDR_IN		    (0x80)	
#define EP_ADDR_OUT		    (0x00)

#define EP_ATTR_CONTROL					(0x0)	
#define EP_ATTR_ISOCHRONOUS	    (0x1)
#define EP_ATTR_BULK				    (0x2)
#define EP_ATTR_INTERRUPT				 (0x3)	


//string descriptor
#define LANGID_US_L 		    (0x09)  
#define LANGID_US_H 		    (0x04)

#define SENT_STALL          BIT(6)




//设备描述符
struct USB_DEVICE_DESCRIPTOR{
    u8 bLength;    
    u8 bDescriptorType;         
    u8 bcdUSBL;
    u8 bcdUSBH;
    u8 bDeviceClass;          
    u8 bDeviceSubClass;          
    u8 bDeviceProtocol;          
    u8 bMaxPacketSize0;         
    u8 idVendorL;
    u8 idVendorH;
    u8 idProductL;
    u8 idProductH;
    u8 bcdDeviceL;
    u8 bcdDeviceH;
    u8 iManufacturer;
    u8 iProduct;
    u8 iSerialNumber;
    u8 bNumConfigurations;
}__attribute__((packed));

//配置描述符
struct USB_CONFIGURATION_DESCRIPTOR{
    u8 bLength;    
    u8 bDescriptorType;         
    u8 wTotalLengthL;
    u8 wTotalLengthH;
    u8 bNumInterfaces;
    u8 bConfigurationValue;
    u8 iConfiguration;
    u8 bmAttributes;
    u8 maxPower;          
}__attribute__((packed));
    
//端点描述符
struct USB_INTERFACE_DESCRIPTOR{
    u8 bLength;    
    u8 bDescriptorType;         
    u8 bInterfaceNumber;
    u8 bAlternateSetting;
    u8 bNumEndpoints;
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
    u8 iInterface;
}__attribute__((packed));

//端点描述符
struct USB_ENDPOINT_DESCRIPTOR{
    u8 bLength;    
    u8 bDescriptorType;         
    u8 bEndpointAddress;
    u8 bmAttributes;
    u8 wMaxPacketSizeL;
    u8 wMaxPacketSizeH;
    u8 bInterval;
}__attribute__((packed));

typedef struct _HEADER_FUNCTIONAL_DESCRIPTOR
{
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bcdCDCL;
	u8 bcdCDCH;
}__attribute__((packed))HEADER_FUNCTIONAL_DESCRIPTOR;
//
typedef struct _CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR
{
		u8 bFunctionLength;
		u8 bDescriptorType;
		u8 bDescriptorSubtype;
		u8 bmCapabilities;
		u8 bDataInterface;
		//u8 bDataInterface1;
}__attribute__((packed))CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR;
//
typedef struct _ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR
{
		u8 bFunctionLength;
		u8 bDescriptorType;
		u8 bDescriptorSubtype;
		u8 bmCapabilities;
}__attribute__((packed))ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR;

typedef struct _UNION_FUNCTIONAL_DESCRIPTOR
{
		u8 bFunctionLength;
		u8 bDescriptorType;
		u8 bDescriptorSubtype;
		u8 bMasterInterface;
		u8 bSlaveInterface;
}__attribute__((packed))UNION_FUNCTIONAL_DESCRIPTOR;

typedef struct _EP_DATA_HANDLE
{
		u8 *buff;
		u16 buff_size;
		u16 data_size;
		u16 cnt;
		u8 ep_attr;
		u8 direction;
		u8 ep_size;
		u8 index;
		u8 start_flag;
		volatile  u8 busy;
		c_buff_struct *cbuff;
		u8 (*ep_isr_deal)(u8);
}ep_data_handle;

typedef enum
{
		SIZE_8 =0x01,
		SIZE_16,
		SIZE_32,
		SIZE_64,
}EP_MAXSIZE_ENUM;
/*typedef struct _EP_CONF_HANDLE
	{
	EP_MAXSIZE_ENUM ep_size;
	u8 ep_attr;
	u8 ep_index;
	u8 direction;
	}ep_conf_hadle;
	*/
typedef enum 
{
		NORM =0x00,
		CLASS,
		USER,
}cmd_type;


typedef enum 
{
		DEVICE=0x00,
		INTERFACE,
		ENDPOINT,
		OTHER,
}recive_type;

typedef struct _request_type
{	
		u8 recive_type:5;
		u8 cmdtype:2;
		u8 dir :1; 
}__attribute__((packed)) request_type;
typedef struct _USB_SETUP_DATA{
		request_type bmRequestType;    
		u8 bRequest;         
		u8 bValueL;          
		u8 bValueH;          
		u8 bIndexL;          
		u8 bIndexH;          
		u16 bLength;                 
}USB_SETUP_DATA;


typedef struct _usb_drv_io
{
		u8 (*ep_write)(u8 *, u16, u8);
		u8 (*data_arrive_callback) (u8 * buff, u16 size, u8 ep_index);
}_usb_drv_io;

typedef struct _usb_drv_descriptor
{
		struct USB_DEVICE_DESCRIPTOR * device_descriptor_point;
		u8 * configuration_descriptor_point;
		u8 **string_descriptor_point;
		u8 * interface_descriptor_point;
}_usb_drv_descriptor;


typedef struct _usb_drv_handle
{
		_usb_drv_io io;
		_usb_drv_descriptor descriptor;
		u8 *usb_name;
		u8 status;
}_usb_drv_handle;


typedef enum
{
		NO_ERR = 0x00,
		UNKNOWN_CMD,
		MEM_ERR,
		EP_INDEX_ERR,
		ARG_ERR,
		DESCRIPTOR_ERR,
		NO_BUFF,
		HANDLE_ERR,
		DATA_END,
}USB_ERR_ENUM;

#define EP_BUFF_MAX					1024  //除端点0外 其他端点的缓存区大小
#define EP0_BUFF_MAX       (0x08)

#define set_data_end()          EP0_CSR |= BIT(3)
#define set_data_end_and_in()   EP0_CSR |= BIT(3)|BIT(1)

//void init_usb(void);

u8 write_ep(u8 *buff, u16 len, u8 ep);
u8 open_usb(_usb_drv_handle *usb_hd, u8(*receive_callback) (u8 *, u16, u8), u8(*class_request_cb) (USB_SETUP_DATA *));
void close_usb();
u8 descriptor_register(u8 descriptor_type, void * descriptor, u8 index);
void set_get_ep0_data(void);
u16 get_ep_buff_free(u8 ep_index);
void unload_usb();

void interrupt_mode_callback_register(u8 (*cb_func)(u8, void *, u16));

#endif
