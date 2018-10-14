#ifndef _S3C2440_UART
#define _S3C2440_UART
#include "c_buff.h"
typedef struct UART_REG
{
	volatile int ULCON;
	volatile int UCON;
	volatile int UFCON;
	volatile int UMCON;
	volatile int UTRSTAT;
	volatile int UERSTAT;
	volatile int UFSTAT;
	volatile int UMSTAT;
	volatile int UTXH;
	volatile int URXH;
	volatile int UBRDIV;
}UART_REG;

/*
typedef struct rx_package
{
	char *buff;
	u8 size;
	struct rx_package  *next_package;
}rx_package;
typedef struct uart_rx_list
{
	rx_package *package;
	u8 package_cnt;
	u8 rx_flag;
}uart_rx_list;
*/
typedef struct _uart_io
{
		void (*putbyte)(char c);
		void (*puts)(void *buff, u16 size);
		u16 (*read)(void *buff, u16 size);
}uart_io;

typedef struct _uart_handle
{
		c_buff_struct *read_cbuff;
		c_buff_struct *write_cbuff;
		uart_io io;
		u8 package_cnt;  //读取缓存里有几包数据
}uart_handle;
#define UART0  ((UART_REG *)0x50000000)
#define UART1  ((UART_REG *)0x50004000)
#define UART2  ((UART_REG *)0x50008000)
#define INTMSK  	*((volatile int *)0X4A000008)
#define INTSUBMSK *((volatile int *)0X4A00001C)
#define SUBSRCPND *((volatile int *)0X4A000018)

#define RX_FIFO_SIZE  	31
#define TX_FIFO_SIZE		63	

#define CBUFF_MAX_SIZE  512
typedef enum
{
	UART0_SERIAL = 0,
	UART1_SERIAL,
	UART2_SERIAL ,
}USERIAL;

uart_handle *uart_open(USERIAL serial, u32 Baud);
void uart_putbyte(USERIAL serial, char byte);
void uart_write(USERIAL serial, char *buff, u8 size);
u8 uart_read(USERIAL serial, char *buff);
uart_handle *get_uart_hd(USERIAL serial);
#endif
