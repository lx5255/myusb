#ifndef _SCSI_
#define _SCSI_
typedef struct cdb_cmd_6b
{
	u8 operation_code;
	u8 logic_block_addr_2;
	u8 logic_block_addr_1;
	u8 logic_block_addr_0;
	u8 length;
	u8 control;
}cdb_cmd_6b;

typedef struct cdb_cmd_10b
{
	u8 operation_code;
	u8 service_action;
	u8 logic_block_addr_3;
	u8 logic_block_addr_2;
	u8 logic_block_addr_1;
	u8 logic_block_addr_0;
	u8 reserved;
	u8 length_H;
	u8 length_L;
	u8 control;
}cdb_cmd_10b;

typedef struct cdb_cmd_12b
{
	u8 operation_code;
	u8 service_action;
	u8 logic_block_addr_3;
	u8 logic_block_addr_2;
	u8 logic_block_addr_1;
	u8 logic_block_addr_0;
	u8 length_3;
	u8 length_2;
	u8 length_1;
	u8 length_0;
	u8 control;
}cdb_cmd_12b;

enum
{
	CMD_ERR = 0X01,
};

  // The standard INQUIRY data shall contain at least 36 bytes
// This is the reduced structure for Mass Storage Devices
typedef struct
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

typedef struct 
{
    u8 peripheral;        // Device Type
    u8 page_code;
    u8 RESERVED;
    u8 page_len;
    u8 page_num[10];
} _Unit_Serial_Number;

typedef struct //���ģʽ
{
    u32  list_length;
    u32 block_num;  
    u32 block_size; 
}read_format_resp;

typedef struct //���ģʽ
{
    u32 last_block_addr;
    u32 block_size;
}Read_Capacity_resp;



    //UFI ��������
#define Format_Unit 0x04 //��ʽ����Ԫ
#define Inquiry 0x12 //��ѯ
#define Star_Stop 0x1B //����/ֹͣ
#define Mode_Select 0x55 //ģʽѡ��
#define Mode_Sense 0x5A //ģʽ�б�
#define Medium_Removal 0x1E //��ֹ/��������Ƴ�
#define Read_10 0x28 //��10
#define Read_12 0xA8 //��12
#define Read_Capacity 0x25 //������
#define Read_Format_capacity 0x23 //����ʽ������
#define Request_Sense 0x03 //�����ж�
#define Rezero_Unit 0x01 //��0
#define Seek_10 0x2B //Ѱ��10
#define Send_Diagnostic 0x1D //�������
#define Test_Unit_Ready 0x00 //���Ե�Ԫ׼��
#define Verify 0x2F //��֤
#define Write_10 0x2A //д10
#define Write_12 0xAA //д12
#define Write_and_Verify 0x2E //д����֤
#define SENSE6 0x1a  //


//#define be_to_le_u16(u16w)    (u16w>>8|u16w<<8)

u8  scsi_cmd_deal(u8 *cmd, u8 len);

#endif
