//#include "typedef.h"
//#include "sdk_cfg.h"
#include "hid.h"
//#include "bluetooth/le_hid_server.h"

static _report_send_func report_send_fun = NULL;
static const u8 hidReportMap[] =
{
  0x05, 0x01,  // UsagePage (Generic Desktop)

  0x09, 0x02, // Usage (Mouse)

  0xA1, 0x01, // Collection (Application)

  0x85, 0x01, // Report Id (1)

  0x09, 0x01, //   Usage (Pointer)

  0xA1, 0x00, //   Collection (Physical)

  0x05, 0x09, //     Usage Page (Buttons)

  0x19, 0x01, //     Usage Minimum (01) -Button 1

  0x29, 0x03, //     Usage Maximum (03) -Button 3

  0x15, 0x00, //     Logical Minimum (0)

  0x25, 0x01, //     Logical Maximum (1)

  0x75, 0x01, //     Report Size (1)

  0x95, 0x03, //     Report Count (3)

  0x81, 0x02, //     Input (Data, Variable,Absolute) - Button states

  0x75, 0x05, //     Report Size (5)

  0x95, 0x01, //     Report Count (1)

  0x81, 0x01, //     Input (Constant) - Paddingor Reserved bits

  0x05, 0x01,  //     Usage Page (GenericDesktop)

  0x09, 0x30, //     Usage (X)

  0x09, 0x31, //     Usage (Y)

  0x09, 0x38, //     Usage (Wheel)

  0x15, 0x81, //     Logical Minimum (-127)

  0x25, 0x7F, //     Logical Maximum (127)

  0x75, 0x08, //     Report Size (8)

  0x95, 0x03, //     Report Count (3)

  0x81, 0x06, //     Input (Data, Variable,Relative) - X & Y coordinate

  0xC0,       //   End Collection

  0xC0,       // End Collection

  0x05, 0x0c,//USAGE_PAGE (Consumer Devices)	05 0C

  0x09, 0x01, //USAGE (Consumer Control)	09 01

  0xa1, 0x01,//COLLECTION (Application)	A1 01 

  0x85, 0x02, //REPORT_ID (2)	85 02 
  
  0x09, 0x86, //USAGE (Channel)	09 86
  
  0x09, 0xe0, //USAGE (Volume)	09 E0
  
  0x15, 0xff,//LOGICAL_MINIMUM (-1)	15 FF 
  
  0x25, 0x01, //LOGICAL_MAXIMUM (1)	25 01 
  
  0x75, 0x02 ,//REPORT_SIZE (2)	75 02 
  
  0x95, 0x02, //REPORT_COUNT (2)	95 02 
  
  0x81, 0x06, //INPUT (Data,Ary,Rel)	81 04 

  0x09, 0xcd, //USAGE (Volume)	09 E0

  0x09, 0xb5,

  0x09, 0xb6,

  0x09, 0xb9,
  
  0x15, 0x00,//LOGICAL_MINIMUM (-1)	15 FF 
  
  0x25, 0x01, //LOGICAL_MAXIMUM (1)	25 01 
  
  0x75, 0x01 ,//REPORT_SIZE (2)	75 02 
  
  0x95, 0x04, //REPORT_COUNT (2)	95 02 
  
  0x81, 0x06, //INPUT (Data,Ary,Rel)	81 04 
 
  0xc0, //END_COLLECTION	C0
};

static u32 read_report_map_callback(u16 offset, void *buffer, u16 len);
static u32 output_report_callback(u8 report_id, void *data, u16 len);
/*----------------------------------------------------------------------------*/
/** @brief: hid模块初始化 
    @param: null
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void _hid_init()
{
#if HID_TYPE == BLE_HID
    report_send_fun = le_hid_input_report_send;
    register_le_hid_callback(read_report_map_callback, output_report_callback);
#elif HID_TYPE == USB_HID

#endif
}
static u32 read_report_map_callback(u16 offset, void *buffer, u16 len)
{
    u16 size = sizeof(hidReportMap);
    if(size>len)
    {
        uprintf("read buff over %d-%d\n", len, size);
        size = len;
    }
    memcpy(buffer,&hidReportMap[offset], size);
    return size;
}
inline u32 get_report_map_size()
{
    return  sizeof(hidReportMap);
}
//输出报告回调
static u32 output_report_callback(u8 report_id, void *data, u16 len)
{
    return 0;
}
//输入报告发送
inline u32 input_report_send(u8 report_id, void *data, u16 len)
{
    if(report_send_fun)
        return report_send_fun(report_id, data, len);
    return 0;
}
/*----------------------------------------------------------------------------*/
/** @brief: null
    @param: null
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
//发送鼠标的数据
static s8 mouse_data[4];
u32 send_mouse(s8 key, s8 x,s8 y, s8 wheel)
{
   int ret_val;
   mouse_data[0] = key;
   mouse_data[1] = x;
   mouse_data[2] = y;
   mouse_data[3] = wheel;
   return input_report_send(1, mouse_data, sizeof(mouse_data));
}

static _volume_data volume_data;
/*u32 send_volume(s8 Channel, s8 volume, u8 music_pp)
{
    int ret_val;
    volume_data.channel = Channel;
    volume_data.Volume = volume;
    volume_data.music_pp = music_pp;
    volume_data.const_data = 0;
    return input_report_send(2, &volume_data, sizeof(volume_data));
}
*/
//设置鼠标灵敏度
static u8 mouse_snes = 10;
void set_mouse_sends(u8 data)
{
    mouse_snes = data;
    if(mouse_snes == 0) 
        mouse_snes = 1;      
    if(mouse_snes > 127)
        mouse_snes = 127;
}
/*----------------------------------------------------------------------------*/
/** @brief: 鼠标控制 
    @param: null
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void mouse_control(u8 cmd)
{
    switch(cmd)
    {
        case MOUSE_UP:
            send_mouse(0, 0, mouse_snes, 0); 
            break;

        case MOUSE_DOWN:
            send_mouse(0, 0, -mouse_snes, 0); 
            break;

        case MOUSE_RIGHT:
            send_mouse(0, mouse_snes, 0, 0); 
            break;

        case MOUSE_LEFT:
            send_mouse(0, -mouse_snes, 0, 0); 
            break;

        case WHEEL_UP:
            send_mouse(0, 0, 0, mouse_snes);
            break;

        case MOUSE_LEFT_BUTTON:
            send_mouse(0x01, 0, 0, 0);
            send_mouse(0, 0, 0 ,0);
            break;

        case MOUSE_MIDDLE_BUTTON:
            send_mouse(0x02, 0, 0, 0);
            send_mouse(0, 0, 0 ,0);
            break;

        case MOUSE_RIGHT_BUTTON:
            send_mouse(0x04, 0, 0, 0);
            send_mouse(0, 0, 0 ,0);
            break;

        default:
            break;
    }
}

/*----------------------------------------------------------------------------*/
/** @brief:hid 音乐控制指令 
    @param: null
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void hid_music_control(u8 cmd)
{
   *(u8 *)&(volume_data)  = 0;
   switch(cmd)
   {
       case VOLUME_UP:
   //         send_volume(0, 1, 0);
            volume_data.Volume = 1;
            break;

       case VOLUME_DOWN:
            //send_volume(0, -1, 0);
            volume_data.Volume = -1;
            break;

       case MUSIC_PP:
            //send_volume(0, 0, 1);
            volume_data.music_pp = 1;
            break;

       case MUSIC_NEXT:
            volume_data.muisc_next = 1;
            break;

       case MUSIC_PRE:
            volume_data.music_pre = 1;
            break;

       case MUSIC_RANDOM:
            volume_data.music_random = 1;
            break;

       default:
            break;

   } 
   input_report_send(2, &volume_data, sizeof(volume_data));
}
