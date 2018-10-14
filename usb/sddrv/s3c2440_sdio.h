#ifndef _SDIO_
#define _SDIO_

//#define SD_DEBUG
#ifdef SD_DEBUG
#define sd_printf		uprintf
#else
#define sd_printf(...)
#endif
typedef struct sdio_reg
{
   volatile int CON;
   volatile int PRE;   //分频系数
   volatile int CMDARG;
   volatile int CMDCON;
   volatile int CMDSTA;
   volatile int RSP0;	//响应寄存器0
   volatile int RSP1;
   volatile int RSP2;
   volatile int RSP3;
   volatile int TIMER;	//超时时间
   volatile int BSIZE;
   volatile int DATCON;
   volatile int DATCNT;
   volatile int DATSTA;
   volatile int FSTA;
   volatile int INT_MSK;
	volatile int DAT;
} sdio_reg;

#define SDIO		((volatile sdio_reg *)0x5A000000)
#define SDIDATB *((volatile u8 *)0x5A000048)
#define GPECON  *((volatile unsigned int *)0x56000040)
#define GPEUP		*((volatile unsigned int *)0x56000048)

#define BLOCK_SIZE      (512)
#define DATASIZE_U32   (0x02<<22)
#define DATASIZE_U16   (0x01<<22)
#define DATASIZE_U8     (0)

#define BLKMODE_BLOCK    (1<<17)
#define BLKMODE_FLUX    (0<<17)

#define WIDEBUS_4BIT    (1<<16)
#define WIDEBUS_1BIT    (0<<16)

#define EnDMA           (1<<15)
#define DisDMA           (0<<15)

#define DATA_START    (1<<14)

#define SEND_DATA_MODE      (3<<12)
#define RECIVE_DATA_MODE    (2<<12)
#define CHECK_BUSY          (1<<12)


#define NOBUSY_INT          (1<<18)
#define RSPCRC_INT          (1<<17)
#define CMD_SEND_INT        (1<<16)
#define CMDTIMEOUT_INT      (1<<15)
#define RSPEND_INT          (1<<14)
#define RWREQ_INT           (1<<13)
#define CRCSTA_INT          (1<<10)
#define RECIBE_CRC_ERR_INT  (1<<9)
#define DATATIMEOUT_INT     (1<<8)
#define DATAFIN_INT         (1<<7)
#define BUSYFIN_INT         (1<<6)
#define TFHALF_INT          (1<<4)
#define TFEMPT_INT          (1<<3)
#define RTLAST_INT          (1<<2)
#define REFULL_INT          (1<<1)
#define REHAFL_INT          (1<<0)


//data sta
#define NOBUSY_STA          (1<<11)
#define CRCERR_STA          (1<<7)
#define DATACRC_STA         (1<<6)
#define DATATIMEOUT_STA     (1<<5)
#define DATFIN_STA          (1<<4)
#define BUSYTFIN_STA        (1<<3)
#define TXDATAON            (1<<1)
#define RXDATAON            (1<<0)


//cmd sta
#define RSPCRC_ERR          (1<<12)
#define CMD_SEND_END        (1<<11)
#define CMDTOUT             (1<<10)
#define RSPFIN              (1<<9)
#define CMD_ON              (1<<8)

//FIFO STA
#define FF_FAIL             (3<<14)
#define TF_DET              (1<<13)
#define RF_DET              (1<<12)
#define TF_HALF             (1<<11)
#define TF_EMPT             (1<<10)
#define RF_LAST             (1<<9)
#define RF_FULL             (1<<8)
#define RF_HAFL             (1<<7)

//res sta 
#define CRC_ERR            (1<<8)
#define TOUT                (1<<7)
#define CMD_MATCH           (1<<6)
#define SD_BUSY             (1<<5)

typedef struct sdio_cmd
{
		u8 cmd;
		bool abortcmd;  //是否终止命令
		bool withdata;  //，命令是否带数据
		u8 rsp_type;  // 0 不响应    1：短响应      2：长响应
		u32 arg;   //命令的参数
}sdio_cmd;

typedef struct sdio_handle
{
    sdio_cmd cmd;
    u16 data_len;
    u16 data_cnt;
    u32 rsp[4];
    u8 *buff;
    u8 data_dir;  //0 send    1 recive
    u8 sta_flag;  
} sdio_handle;

void sdio_init();
u8 sdio_cmd_send(sdio_handle *hd);
u8 open_sdio(sdio_handle *hd);
void sdio_exit();
inline u8 check_sdio_busy(void);
inline void set_sdio_clk(u8 pre);

#endif // _SDIO_
