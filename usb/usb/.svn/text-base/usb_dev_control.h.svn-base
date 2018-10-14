#ifndef _USB_DEV_CONTR_
#define _USB_DEV_CONTR_

#define control_data_end()          {EP0_CSR |= BIT(3);}
#define control_data_in_end()				 {EP0_CSR |= BIT(3)|BIT(1);}

#define EP_FIFO_8		(1<<0)
#define EP_FIFO_16  (1<<1)
#define EP_FIFO_32  (1<<2)
#define EP_FIFO_64  (1<<3)

#define BULK_MODE		 (0<<6) 

#define IN_MODE					(1<<5)	
#define OUT_MODE				(0<<5)	

#define USB_RES					BIT(2)
#define USB_RESUME      BIT(1)
#define USB_SUSPEND     BIT(0)
#define EP0_SETUP_END		BIT(4)
#define EP0_DATA_END		BIT(2)
#define EP0_OUT_RDY     BIT(0)
#define EP_IN_SENT_STALL BIT(5)
#define EP_IN_FLUSH_FIFO BIT(3)
#define EP_IN_RDY				 BIT(0)
#define EP_OUT_RDY			 BIT(0)		
#define EP_OUT_SENT_STALL BIT(6)
#define EP_OUT_FLUSH_FIFO BIT(4)


#define ep0_clear_out_rdy()  do{EP0_CSR |= BIT(6);}while(0)
#define ep_clear_out_rdy()  do{OUT_CSR1_REG &= ~BIT(0);}while(0)

#define EP_ATTR_CONTROL					(0x0)	
#define EP_ATTR_ISOCHRONOUS	    (0x1)
#define EP_ATTR_BULK				    (0x2)
#define EP_ATTR_INTERRUPT				 (0x3)	

typedef void (*usb_event_callback)(u16 evnt, u8 ep_index, u8 *buffer, u8 len);

enum 
{
		RES_EVENT = 0x00,
		RESUME_EVENT,
		SUSPEND_EVENT,
		DATA_OUT_EVENT,
		DATA_IN_EVENT,
};

enum
{
		CLEAR_SETUP_END = 0x00,
		FLUSH_EP,
		CLEAR_DATA_END,
		CLEAR_SENT_STALL,
		GET_EP_DIR,
		GET_EP_SIZE,
		GET_EP_SEND_STATUS,
};
u32 usb_hw_ctrl(u8 CMD, u32 data);
void init_usb_dev_control(usb_event_callback event_cb);
void close_usb_dev_control(void);
u8 write_ep_fifo(u8 ep_index, u8 *buff, u8 size); 
u8 open_ep(u8 ep_index, u8 ep_size, u8 dir, u8 mode);
void close_ep(u8 ep_index);
#endif
