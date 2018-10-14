#include "usb_dev_enum.h"
#include "SCSI.h"
#include "c_buff.h"
#include "task_manage.h"
#include "cmd.h"

_usb_drv_handle usb_hd;
#if USB_DEV_ENUM == USB_COM
const struct USB_DEVICE_DESCRIPTOR device_descriptor = //�豸������
{
		sizeof(struct USB_DEVICE_DESCRIPTOR),//��������С
		DEVICE_TYPE,
		0x10,
		0x01,//USB�汾Ϊ1.1 
		0x02,//�豸�����룬ͨ���豸��
		0x00,//��������
		0x00,//�豸Э�����
		0x08,//�˵�0�����С8
		0x66,
		0x66,//���̴���
		0x78,
		0x23,//��Ʒ���
		0x00,
		0x01,//�������
		0x00,//�豸�����ַ�������
		0x00,//������Ʒ�ַ�������
		0x00,//�����豸���к�����
		0x01,//���ܵ���������
};


const CONF_INT_ENDP_DESCRIPTOR config =
{
	{ //��������
		sizeof(struct USB_CONFIGURATION_DESCRIPTOR),
		CONFIGURATION_TYPE,
		sizeof(CONF_INT_ENDP_DESCRIPTOR) % 256,//32���ֽ�
		sizeof(CONF_INT_ENDP_DESCRIPTOR) / 256,//sizeof(CONF_INT_ENDP_DESCRIPTOR)/256,
		0x02,//ֻ��һ���ӿ�
		0x01,//���ñ��
		0,
		0xa0,
		25 //������50mA
	},

	//CDC interface_descritor //CDC ��ӿ�������
	{
		sizeof(struct USB_INTERFACE_DESCRIPTOR),//�������ֽڴ�С
		INTERFACE_TYPE,//�ӿ����������ͱ��
		0x00,//�ӿڱ��Ϊ0
		0x00,//�ýӿ��������ı��ñ��
		0x00,//��0 �˵�����Ϊ1���˵�1 �ж�
		0x02,//��ʹ�õ��࣬Communication Interface Class
		0x02,//ʹ�õ����࣬Abstract Control Model
		0x01,//ʹ�õ�Э�飬Common AT commands
		0x00 //�ӿ��������ַ�������
	},

	//header_functional_descriptor
	{
		sizeof(HEADER_FUNCTIONAL_DESCRIPTOR),//Size of this descriptor in bytes
		CS_INTERFACE_DESCRIPTOR,//CS_INTERFACE descriptortype
		0x00,//Header functional descriptor subtype
		0x10, //CDC �汾��
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
		0x00,//MasterInterface��CDC �ӿ�
		0x01 //SlaveInterface�����ݽӿ�
	},

	//endpoint1in_descriptor
	/*{
		sizeof(struct USB_ENDPOINT_DESCRIPTOR),//�������ֽڴ�С
		ENDPOINT_TYPE,//����������
		3 | EP_ADDR_IN,//1������˵�
		EP_ATTR_INTERRUPT,
		0x20,
		0x00,
		0x80,
	},*/
	{ //�ӿ�������
		sizeof(struct USB_INTERFACE_DESCRIPTOR),//�������ֽڴ�С
		INTERFACE_TYPE,//����������
		0x01,//�ӿڱ��
		0x00,//���ñ��
		0x02,//��0�˵���
		0x0A,//ʹ�õ���
		0x00,//ʹ�õ�����
		0x00,//ʹ�õ�Э��
		0x00 //�ӿ������ַ�������
	},
	{
		{
			sizeof(struct USB_ENDPOINT_DESCRIPTOR),//�������ֽڴ�С
			ENDPOINT_TYPE,//����������
			2 | EP_ADDR_IN,//1������˵�
			EP_ATTR_BULK,
			0x20,
			0x00,
			0x00,
		},
		{
			sizeof(struct USB_ENDPOINT_DESCRIPTOR),//�������ֽڴ�С
			ENDPOINT_TYPE,//����������
			1 | EP_ADDR_OUT,//1������˵�
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
		**	function	��USB ��������
		**	argument	��
		**	return	�� 	�����
		**	note		��
-------------------------------------------------------------------*/
u8 usb_write(void*buff,u16 len)
{
	return usb_hd.io.ep_write(buff,len,2);
}


/*---------------------------------------------------------------
		**	function	��USB���ݽ��ջص�
		**	argument	��read_data �����յ������� size ���ݴ�С
							ep_index �˵��
		**	return	��    
		**	note		��
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
		**	function	��������ص�
		**	argument	��cmd ��������
		**	return	��    0 ���� data_end������
						 1 ���� �հ�������
						 2 ������һ��ep0�����ݣ��ӽ��ջص�����
						 other ������������
		**	note		��
-------------------------------------------------------------------*/
u8 class_callback(USB_SETUP_DATA*cmd)
{
	// uprintf("c 0x%x\n\r", cmd->bRequest);
	if(cmd->bmRequestType.dir==1) //����� �豸������
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
		**	function	����ʼ�����⴮��
		**	argument	��void
		**	return	��    void
		**	note		��
-------------------------------------------------------------------*/
u8 open_usb_uart(void)
{
	descriptor_register(DEVICE_TYPE,(u8*)&device_descriptor,0); //ע���豸������
	descriptor_register(CONFIGURATION_TYPE,(u8*)&config,0); //ע������������
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
const struct USB_DEVICE_DESCRIPTOR device_descriptor = //�豸������
{
	sizeof(struct USB_DEVICE_DESCRIPTOR),//�豸���������ֽ�����С
	DEVICE_TYPE,//�豸���������ͱ��
	0x10,//USB �汾��
	0x01,
	0x00,//USB ������豸�����
	0x00,//USB ������������
	0x00,//USB ������豸Э�����
	0x08,//�˵�0 ��������С
	0x88,//���̱��
	0x88,
	0x05,//��Ʒ���
	0x00,
	0x00,//�豸�������
	0x01,
	0x01,//�豸�����ַ���������
	0x02,//������Ʒ�ַ���������
	0x00,//�����豸���к��ַ���������
	0x01 //���ܵ���������
};


const CON_INT_ENDP_DESCRIPTOR_STRUCT con_int_endp_descriptor =
{
	//configuration_descriptor //����������
	{
		sizeof(struct USB_CONFIGURATION_DESCRIPTOR),//�������������ֽ�����С
		CONFIGURATION_TYPE,//�������������ͱ��
		sizeof(CON_INT_ENDP_DESCRIPTOR_STRUCT)%256,
		sizeof(CON_INT_ENDP_DESCRIPTOR_STRUCT)/256,
		0x01,//ֻ����һ���ӿ�
		0x03,//�����õı��
		0x00,//iConfiguration �ֶ�
		0xc0,//�������߹��磬��֧��Զ�̻���
		0x30 //�����߻�ȡ������400mA
	},

	//interface_descritor //�ӿ�������
	{
		sizeof(struct USB_INTERFACE_DESCRIPTOR),//�ӿ����������ֽ�����С
		INTERFACE_TYPE,//�ӿ����������ͱ��
		0x00,//�ӿڱ��Ϊ0
		0x00,//�ýӿ��������ı��
		0x02,//��0 �˵�����Ϊ2��ֻʹ�ö˵����˵��������� 
		0x08,//USB �������洢�豸
		0x06,//ʹ�õ����࣬Ϊ�򻯿�����
		0x50,//ʹ�õ�Э�飬����ʹ�õ���������Э��
		0x02 //�ӿ��������ַ�������
	},

	//endpoint_descriptor[]
	{
		{
			sizeof(struct USB_ENDPOINT_DESCRIPTOR),//�������ֽڴ�С
			ENDPOINT_TYPE,//����������
			2 | EP_ADDR_IN,//1������˵�
			EP_ATTR_BULK,
			0x40,
			0x00,
			0x00,
		},
		{
			sizeof(struct USB_ENDPOINT_DESCRIPTOR),//�������ֽڴ�С
			ENDPOINT_TYPE,//����������
			1 | EP_ADDR_OUT,//1������˵�
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
    if(cbw_cmd->dCBWDataTransgerLength == 0)  //����������
    {
        csw_step = CSW_END;
    }
    else //��������
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
		**	function	��USB ��������
		**	argument	��
		**	return	�� 	�����
		**	note		��
-------------------------------------------------------------------*/
u8 usb_write(void *buff,u16 len)
{
	return usb_hd.io.ep_write(buff,len,2);
}

/*---------------------------------------------------------------
		**	function	��������ص�
		**	argument	��cmd ��������
		**	return	��    0 ���� data_end������
						 1 ���� �հ�������
						 2 ������һ��ep0�����ݣ��ӽ��ջص�����
						 other ������������
		**	note		��
-------------------------------------------------------------------*/
const u8 store_num = 0;
u8 class_callback(USB_SETUP_DATA*cmd)
{
	 
	if(cmd->bmRequestType.dir==1) //����� �豸������
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
		**	function	����ʼ���������豸
		**	argument	��void
		**	return	��    void
		**	note		��
-------------------------------------------------------------------*/

u8 open_usb_disk(void)
{
	descriptor_register(DEVICE_TYPE,(u8*)&device_descriptor,0); //ע���豸������
	descriptor_register(CONFIGURATION_TYPE,(u8*)&con_int_endp_descriptor,0); //ע������������
	descriptor_register(STRING_TYPE,(u8*)&sring3,1);
	descriptor_register(STRING_TYPE,(u8*)&sring3,2);
	descriptor_register(STRING_TYPE,(u8*)&sring1,0);
	open_usb(&usb_hd,usb_data_callback,class_callback);

	return 0;
}

#endif

#if USB_DEV_ENUM == USB_HID

const struct USB_DEVICE_DESCRIPTOR device_descriptor = //�豸������
{
	sizeof(struct USB_DEVICE_DESCRIPTOR),//�豸���������ֽ�����С
	DEVICE_TYPE,//�豸���������ͱ��
	0x10,//USB �汾��
	0x01,
	0x00,//USB ������豸�����
	0x00,//USB ������������
	0x00,//USB ������豸Э�����
	0x08,//�˵�0 ��������С
	0x88,//���̱��
	0x88,
	0x05,//��Ʒ���
	0x00,
	0x00,//�豸�������
	0x01,
	0x01,//�豸�����ַ���������
	0x02,//������Ʒ�ַ���������
	0x00,//�����豸���к��ַ���������
	0x01 //���ܵ���������	
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
	//configuration_descriptor //����������
	{
			sizeof(struct USB_CONFIGURATION_DESCRIPTOR), //�������������ֽ�����С
			CONFIGURATION_TYPE, //�������������ͱ��
			sizeof(CON_INT_ENDP_DESCRIPTOR_STRUCT)%256,
		  sizeof(CON_INT_ENDP_DESCRIPTOR_STRUCT)/256,
			0x01, //ֻ����һ���ӿ�
			0x01, //�����õı��
			0x00, //iConfiguration �ֶ�
			0xA0, //�������߹��磬֧��Զ�̻���
			0xC8 //�����߻�ȡ������400mA
	},
		//interface_descritor //�ӿ�������
		{ 
				sizeof(struct USB_INTERFACE_DESCRIPTOR), //�ӿ����������ֽ��� ��С
				INTERFACE_TYPE, //�ӿ����������ͱ��
				0x02, //�ӿڱ��Ϊ0
				0x00, //�ýӿ��������ı��
				ENDPOINT_NUMBER, //��0 �˵�����Ϊ2��ֻʹ�� �˵����˵���������
				0x03, //�˻��ӿ��豸��HID����
				0x01, //ʹ�õ����ࣺ֧��BIOS ������Boot ������
				0x01, //ʹ�õ�Э�飺����
				0x00 //�ӿ��������ַ�������
		},
		//hid_descriptor
		{
				sizeof(HID_DESCRIPTOR_STRUCT), //HID ���������ֽ�����С
				HID_DESCRIPTOR, //HID ���������ͱ��
				0x0110, //HID ��Э��汾�ţ�Ϊ1.1
				0x00, //�̼��Ĺ��ҵ������ţ�0x21 Ϊ����
				NUM_SUB_DESCRIPTORS, //�¼�������������
				{
						{
								REPORT_DESCRIPTOR, //�¼�������Ϊ����������
								sizeof(KeyBoardReportDescriptor) //�¼��������ĳ���
						}
				},
		},
		//endpoint_descriptor[]
		{
				{ //���˵���������
						sizeof(struct USB_ENDPOINT_DESCRIPTOR),//�������ֽڴ�С
						ENDPOINT_TYPE,//����������
						1 | EP_ADDR_IN,//1������˵�
						EP_ATTR_INTERRUPT,
//						EP_ATTR_BULK,
						0x08,
						0x00,
						0x0a,

				},
	/*			{ //���˵��������
						sizeof(struct USB_ENDPOINT_DESCRIPTOR),//�������ֽڴ�С
						ENDPOINT_TYPE,//����������
						2 | EP_ADDR_OUT,//2������˵�
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
		**	function	��������ص�
		**	argument	��cmd ��������
		**	return	��    0 ���� data_end������
						 1 ���� �հ�������
						 2 ������һ��ep0�����ݣ��ӽ��ջص�����
						 other ������������
		**	note		��
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
		**	function	����ʼ��HID�豸
		**	argument	��void
		**	return	��    void
		**	note		��
-------------------------------------------------------------------*/
u8 open_usb_hid(void)
{
	descriptor_register(DEVICE_TYPE,(u8*)&device_descriptor,0); //ע���豸������
	descriptor_register(CONFIGURATION_TYPE,(u8*)&con_int_endp_descriptor,0); //ע������������
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

REGISTER_CMD(key, key_do,"����HID����");

#endif

