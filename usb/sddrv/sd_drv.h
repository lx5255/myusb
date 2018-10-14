#ifndef _SD_DRV_
#define _SD_DEV_
#include "config.h"

#define WIN_CACHE		1  //是否使用缓存
#define SD_MALLOC		1  //动态内存分配使能

#define SD_BLOCK_SIZE	512
typedef enum
{
	GO_IDLE_STATE = 0,
	ALL_SEND_CID,
	SEND_RELATIVE_ADDR,
	SELEC_CHIP,
	SEND_CSD,
	SEND_CID,
	READ_DAT_UNTIL_STOP,
	STOP,
	SEND_STATUS,
	GO_INACTIVE_STATE ,
	SET_BLOCKLEN,
	READ_SINGLE_BLOCK,
	READ_MULTIPLE_BLOCK,
	WRITE_BLOCK,
	WRITE_MULTIPLE_BLOCK,
	PROGRAM_CSD,
	SET_WRITE_PROT,
	CLR_WRITE_PROT,
	SEND_WRITE_PROT,
	ERASE_WR_BLK_START,
	ERASE_WR_BLK_END,
	ERASE,
	APP_CMD,
	SD_APP_OP_COND,
	SDIO_SEND_IF_COND,
	CMD1,
	SET_BUS_WIDTH
}CMD_ENUM;


typedef struct sdio_io
{
   void (*init)(void);
   u8 (*read_block)(u8 *);
   u8 (*write_block)(u8 *);
   u8 (*send_cmd)(u8, u32, u32 *);
}sdio_io;

typedef struct sd_io
{
    void (*init)(void);
#if WIN_CACHE
    u8 (*read)(u32 block_num, u16 offset, u8 *buff, u16 buff_len);
    u8 (*write)(u32 block_num, u16 offset, u8 *buff, u16 buff_len);
    u8 (*sync_sd)(void);    
#endif
    u8 (*read_block)(u32 block_num, u8 *buff, u8 wait_flag);
    u8 (*write_block)(u32 block_num, u8 *buff, u8 wait_flag);
}sd_io;
 typedef struct _ID_struct
{
	u8 id;	//制造商id
	u8 PRV[2]; //版本
	u8 MDT[3]; //生产日期
	u8 OID[3];
	u8 PNM[6]; //产品名
	u32 PSN;  //产品序号
}_ID_struct;
typedef struct _sd_arg
{
   u32 rca;
	u32  respond[4]; 
	u32  CSD[4]; 
	u32  CID[4]; 
	_ID_struct ID;
}_sd_arg;

typedef struct _sd_win
{
    u8 *buff;
    u32 block_num;
}sd_win;
typedef struct sd_handle
{
		_ID_struct ID;
		sd_io io;
		u32  CSD[4]; 
		u32  CID[4];
		u32 rca;
		u32 block_num;
		u32 block_size;	
		u32 sd_size;
		u8 data_tansfer;
		u8 init_flag;
}sd_handle;

u8  _sd_init(void);
u8 sd_read_block(u32 block_num, u8 *buff, u8 wait_flag);
u8 sd_write_block(u32 block_num, u8 *buff, u8 wait_flag);
u8 sd_pre_read(u32 block_num);
void printf_sector(u32 block_num);
sd_handle *open_sd(void);
inline u8 get_sd_busy();
void sd_exit();
#if WIN_CACHE
u16 sd_write(u32 block_num, u16 offset, u8 *buff, u16 buff_len);
u16 sd_read(u32 block_num, u16 offset, u8 *buff, u16 buff_len);
u8 sync_sd(void);
#endif
#endif // _SD_DRV_
