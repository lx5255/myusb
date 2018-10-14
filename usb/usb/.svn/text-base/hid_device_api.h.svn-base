#ifndef __HID__
#define __HID__

#define HID_DRV_DEBUG
#ifdef HID_DRV_DEBUG
#define hid_printf uprintf
#else
#define hid_printf(...)   
#endif

#define BLE_HID             1
#define USB_HID             2
#define HID_TYPE       USB_HID

typedef u32 (*_report_send_func)(u8 , void *, u16 );
typedef u32 (*read_report_map_cb)(u16 , void *, u16 );
typedef u32 (*output_report_cb)(u8, void *, u16);
typedef struct _volume_data
{
    s8 channel:2;
    s8 Volume:2;
    u8 music_pp:1;
    u8 muisc_next:1;
    u8 music_pre:1;
    u8 music_random:1;
}__attribute__((packed)) _volume_data;

enum  //鼠标控制命令
{
    MOUSE_LEFT = 0x00,
    MOUSE_RIGHT,
    MOUSE_UP,
    MOUSE_DOWN,
    WHEEL_UP,
    WHEEL_DOWN,
    MOUSE_LEFT_BUTTON,
    MOUSE_RIGHT_BUTTON,
    MOUSE_MIDDLE_BUTTON,
};
void mouse_control(u8 cmd);
void set_mouse_sends(u8 data);

enum   //音乐控制命令 
{

    VOLUME_UP = 0,
    VOLUME_DOWN,
    MUSIC_PP,
    MUSIC_NEXT,
    MUSIC_PRE,
    MUSIC_RANDOM, 
};
void hid_music_control(u8 cmd);

u32 get_report_map_size();
#endif
