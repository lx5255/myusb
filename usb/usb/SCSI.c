#include "usb_drv.h"
#include "usb_dev_enum.h"
#include "SCSI.h"
#include "sd_drv.h"
//#include "string.h"
//#include "stdlib.h"

/*typedef struct
{
  u8 peripheral;        // Device Type
  u8 rmb;               // Removable Media Bit
  u8 version;           // Version Field
  u8 resp_data_format;  // Response Data Format
  u8 additional_len;    // Additional Length
  u8 sccstp;            // SCC Supported (include embedded storage array)
  u8 bque;              // Basic Queuing
  u8 cmdque;            // Command Queuing
  u8 vendor_id[8];
  u8 product_id[16];
  u8 product_rev[4];
} msd_scsi_inq_resp;
*/
#if USB_DEV_ENUM == USB_DISK
    const msd_scsi_inq_resp inrq_resp = {
      0, // Direct Access Block device 0x00
      0x80,          // Device is removable
      0x04,                                // Comply with SPC-2
      0x02,                                // Standard format
      0x20,                                
      0x00,
      0x00,
      0x00,
      {'R','i','s','e','t','e','k',' '}, 
      {'M','a','s','s',' ','S','t','o','r','a','g','e',' ',' ',' ',' '}, 
      {'0','.','0','1'}, 
    };
const  _Unit_Serial_Number Serial_Number = 
{
    0,
    0x80,
    0,
    2,
    {0x66,0x66,'0','0','0','0','0','0','0','0',}
};
static sd_handle *sd_hd = 0;



u8 Inquiry_deal()
{
    //printf_buff((void *)&inrq_resp, sizeof(msd_scsi_inq_resp) );
 //   uprintf("msd_scsi_inq_resp size:%d\n\r",sizeof(msd_scsi_inq_resp));
	
	usb_write((void *)&inrq_resp, 0x24);
    return 0;
}

//读取U盘格式信息
u8 ReadFormatCapacity_deal()
{
    read_format_resp format_list;
    sd_hd = open_sd();
    if(sd_hd == 0)
    {
        return 2;
    }
   // (u8 *)(&format_list)[3] = 0x08;
    format_list.list_length =  be_to_le_u32(0, 8);
    format_list.block_num = be_to_le_u32(0,sd_hd->block_num);
    format_list.block_size = be_to_le_u32(0,sd_hd->block_size);
    format_list.block_size |= 0x02;

   // format_list.block_num, format_list.block_size, sd_hd->block_num, sd_hd->block_size);
    usb_write((void *)&format_list, 0x12);
	 set_csw_step(CSW_END);
	return 0;
}
u8 TestUnitReady_deal()
{
    
	return 0;
}
u8 RequestSense_deal()
{
	return 0;
}
u8 ModeSense6_deal()
{
	return 0;
}
u8  StartStop_deal()
{
	return 0;
}
u8 MediumRemoval_deal()
{
	return 0;
}

//读取U盘大小
u8  ReadCapacity_deal()
{
	Read_Capacity_resp Capacity_resp;
    sd_hd = open_sd();
    if(sd_hd == 0)
    {
        return 2;
    }
    
    Capacity_resp.last_block_addr = be_to_le_u32(0,sd_hd->block_num-1);
    Capacity_resp.block_size = be_to_le_u32(0,sd_hd->block_size);
  //   uprintf("block_num:%d block_size %d    %d-%d\n\r",\
  //  Capacity_resp.last_block_addr, Capacity_resp.block_size, sd_hd->block_num, sd_hd->block_size);
    usb_write((void *)&Capacity_resp, 0x8);
	set_csw_step(CSW_END);
	return 0;
}

//读取数据
u8 Read_deal(u32 block_addr, u32 len)
{
    u8 buff[512];
    u16 usb_free_size;
	u16 cnt = 0;
    u8 res;
	
	// uprintf("read block:%d size:%d\n\r", block_addr, len);
	while(len--)
    {
		cnt = 0;
		
		sd_read(block_addr++, 0, buff, sd_hd->block_size);
		if(len>2)
		{
            sd_pre_read(block_addr);//预读数据
		}
		
		do{
			//	do{
				    usb_free_size = get_ep_buff_free(2); 
				//   }while(usb_free_size < 1024);//< sd_hd->block_size -cnt
				if(usb_free_size > sd_hd->block_size -cnt)
				{
                    usb_free_size = sd_hd->block_size -cnt;
				}
				//uprintf(" free size %d\n\r", usb_free_size);
				res = usb_write((void *)(&buff[cnt]), usb_free_size);
				//uprintf("write res:%d\n\r", res);
				cnt += usb_free_size;

			}while(cnt<sd_hd->block_size);
					
	}   
   set_csw_step(CSW_END);
  while( get_ep_buff_free(2) < 1024)
   {
       ;
   }
	return 0;
}
u8 Write_deal()
{
   
	return 0;
}
u8  Verify_deal()
{
	return 0;
}
u8 ModeSense10_deal()
{
	return 0;
}
u8  SENSE6_deal()
{
    u8 buff[4] = {0x03,0x00,0x80,0x00};
    usb_write((void *)buff, 0x4);
    set_csw_step(CSW_END);  
    return 0;
}

///6位命令处理
u8 cdb_6b_deal(cdb_cmd_6b *cmd)
{
		//uprintf("6op 0x%x\n\r", cmd->operation_code);
		switch(cmd->operation_code)
		{
				case Inquiry:
						set_csw_step(CSW_END);
						if(cmd->logic_block_addr_2&0x01)
						{
								uprintf("1\n\r");
								usb_write((void *)&Serial_Number, Serial_Number.page_len+4);
								return 0;
						}
						return Inquiry_deal();
						break;
				case SENSE6:
						return SENSE6_deal();
						break;
				case   Test_Unit_Ready:
						return TestUnitReady_deal();
						break;
				case Medium_Removal:
						return MediumRemoval_deal();
						break;

				default:
						return CMD_ERR;
						break;
		}
		return 0;
}
//10位命令才处理
u8 cdb_10b_deal(cdb_cmd_10b *cmd)
{
    u32 logic_block_addr;
    u16 length;

  //  uprintf("10op 0x%x\n\r", cmd->operation_code);
    switch(cmd->operation_code)
	{
	    case Read_Format_capacity:
	        return ReadFormatCapacity_deal();
	    break;
	    case Read_Capacity:
	        return ReadCapacity_deal();
	    break;
	    case Read_10 :
	        logic_block_addr = be_to_le_u32((void *)&cmd->logic_block_addr_3, 0); 
	        length = be_to_le_u16((void *)&cmd->length_H, 0);
	        return  Read_deal(logic_block_addr, length);
	    break;
	    case Write_10 :
	    
	    break;
		default:
		return 1;
		break;
	}
    return 0;
}
//scsi命令处理
u8  scsi_cmd_deal(u8 *cmd, u8 len)
{
   //  printf_buff((u8 *)cmd, len);
	switch(len)
	{
		case 6:
		{
		    return 	cdb_6b_deal((cdb_cmd_6b *)cmd);
		}
		break;
		case 10:
		    return 	cdb_10b_deal((cdb_cmd_10b *)cmd);		
		break;
		default :
		return 1;
		break;
	}
	return 0;

}
#endif

