#include "usb_drv.h"
#include "usb_dev_enum.h"
#include "task_manage.h"
#include "sd_drv.h"
//#include "string.h"
#include "c_buff.h"

#ifdef USB_DEV==USB_DISK
u8 usb_task(void *p);
u8 usb_init(void);
u8 usb_exit();
static sd_handle *sd_hd = 0;
c_buff_struct  *cbw_cmd_cbuf = NULL;

task_ls usb_task_ls =
{
    .name = "usb_task",
    .task_func = usb_task,
    .task_init = usb_init,
    .task_exit = usb_exit,
	 .q_msg = 0,
};

static u32 task_run_cnt = 0;;
static u32 msg[4];
static u8 sd_buf[SD_BLOCK_SIZE];
static CBW cbw_cmd;
u8 usb_task(void *p)
{
	u8 res;

    if(task_run_cnt++ == 1000)
    {
//        uprintf(".");
        task_run_cnt = 0;
    }
	memset((u8 *)msg, 0x00, sizeof(msg));
	res = task_pend_msg(msg);
   // if(msg[0] != 0xffffffff)
    {
      //   uprintf("msg addr 0x%x\n\r", msg);
    //     uprintf("err msg %d  %d %d %d\n\r", msg[0], msg[1], msg[2], msg[3]);
    }
		     
	switch(msg[0])
	{
		case CBW_CMD_ARRIVAL:
		  //  uprintf("cmda  %d  msg %d\n\r", cbw_cmd_cbuf->data_len, get_task_msg_num());
		    read_cbuff(cbw_cmd_cbuf, (u8 *)&cbw_cmd, sizeof(CBW));
		    cbw_cmd_deal(&cbw_cmd);
		break;
		default:
		    if(msg[0] != 0xffffffff)
		         uprintf("err msg %d\n\r", msg[0]);
		break;
	}
	
   return 0;
}

u8 usb_init(void)
{
	if(open_usb_disk())
	{
		return 1;
	}

	sd_hd = open_sd();
	if(sd_hd == 0)
	{
		return 2;
	}
	
  cbw_cmd_cbuf = new_cbuff(200);
	if(cbw_cmd_cbuf == NULL)
	{
		return 0;
	}
	
	return 0;
}

u8 usb_exit()
{
	unload_usb();
	free_cbuff(&cbw_cmd_cbuf);
	return 0;
}
#endif
