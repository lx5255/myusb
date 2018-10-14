#include "config.h"
#include "S3C2440.H"
#include "irq.h"
#include "s3c2440_uart.h"
//#include "stdlib.h"
static int *isr_handle_array = (void *)0x33fb06b0;
//串口配置寄存器句柄
static UART_REG *uart_register[3] =
{
	UART0 ,UART1, UART2
};

static uart_handle uart_hd[3];
static void uart0_putbyte(char c);
void uart0_puts(void  *buff, u16 size);
u16 uart0_read_string(char *string, u16 len);
void uart1_putbyte(char c);
void uart1_puts(void  *buff, u16 size);
static void uart0_isr(void);
u32 uart0_read(char *buffer, u32 size);
//void uart1_isr(void);
static void write_tx_fifo(USERIAL serial);
/*
**	函数	：uart_init(u8 serial, u32 Baud)
**	功能	：串口初始化
**	参数	：serial：串口号	Baud：波特率
**	返回值：无
**	备注	：无
*/
uart_handle *uart_open(USERIAL serial, u32 Baud)
{
		u32 baud_div;

		if(UART0_SERIAL == serial) {
				register_irq(INT_UART0, INT_SUB_RXD0|INT_SUB_TXD0, uart0_isr);
		}	
		else if(UART1_SERIAL == serial)
		{
#ifdef BOOT_START	
				//		isr_handle_array[23] = (int )uart1_isr;
#else		
#endif		
				INTMSK &= ~(1<<23);
				INTSUBMSK &= ~(1<<3);
		}

		if(serial >2 ) {
				return NULL;
		}

		uart_hd[serial].read_cbuff = new_cbuff(CBUFF_MAX_SIZE);
		if(uart_hd[serial].read_cbuff == NULL) {
				return NULL;
		}
		uart_hd[serial].write_cbuff = new_cbuff(CBUFF_MAX_SIZE);
		if(uart_hd[serial].write_cbuff == NULL) {
				free_cbuff(&uart_hd[serial].read_cbuff);
				return NULL;
		}

		GPHCON &= ~(0xf<<(4+serial*4)); //配置IO
		GPHCON |= 0xa<<(4+serial*4);

		baud_div = PCLK/(Baud*16) - 1;	
		uart_register[serial]->UBRDIV = baud_div;	
		uart_register[serial]->UFCON = 0x37;//RXFIFO32个字节触发	TXFIFO 0个字节触发 uart_register[serial]->ULCON = 0x03;
		uart_register[serial]->UCON = 0x185;

		uart_hd[serial].package_cnt = 0;
		if(serial == UART0_SERIAL) {
				uart_hd[serial].io.putbyte = uart0_putbyte;
				uart_hd[serial].io.puts = uart0_puts;
				uart_hd[serial].io.read = uart0_read;
		}
		else if(serial == UART1_SERIAL) {
				uart_hd[serial].io.putbyte = uart1_putbyte;
				uart_hd[serial].io.puts = uart1_puts;
		}
		return &uart_hd[serial];
}

uart_handle *get_uart_hd(USERIAL serial)
{
		return &uart_hd[serial];
}
/*----------------------------------------------------------------
**	函数	：
						void uart0_isr(void)
**	功能	：串口中断处理函数
**	参数	：void
**	返回值：无
**	备注	：无
-------------------------------------------------------------------*/
static void uart0_isr(void)
{	
		if(SUBSRCPND&0x01) {	
				if(UART0->UFSTAT&0x3f) {
						u8 rx_cnt = 0;
						u8 rx_buff[RX_FIFO_SIZE];		

						while((UART0->UFSTAT)&0x3f){ //将FIFO数据拷贝到缓存区
								rx_buff[rx_cnt++] =  UART0->URXH;	
						}	

						rx_cnt = MIN(rx_cnt,get_cbuff_remain(uart_hd[UART0_SERIAL].read_cbuff)-1);
						if(rx_cnt < RX_FIFO_SIZE) {
						//		rx_buff[rx_cnt++] = '\0';  //结束位	
						}

						if(get_cbuff_remain(uart_hd[UART0_SERIAL].read_cbuff)<= rx_cnt){//cbuff已满
								rx_cnt = get_cbuff_remain(uart_hd[UART0_SERIAL].read_cbuff);
						//		rx_buff[rx_cnt-1] = '\0';
						}

						if(rx_buff[rx_cnt-1] == '\0'){//读取完计数
								uart_hd[UART0_SERIAL].package_cnt++;
						}
						write_cbuff(uart_hd[UART0_SERIAL].read_cbuff, rx_buff, rx_cnt);//数据写入缓存
		//								uprintf("rx cnt %d %s\n", rx_cnt, rx_buff);
				}
				UART0->UFCON |= 0x2; 		
				SUBSRCPND |= 0x01;			
		}

		if(SUBSRCPND & 0x02) {
				SUBSRCPND |= 0x02;			
				if(!(UART0->UFSTAT & BIT(14))) {
			//	uprintf("fifo cnt %d\n\r", (UART0->UFSTAT>>8)&0x3f);
//						if(((UART0->UFSTAT>>8)&0x3f) < check_fifo_cnt){
//								if((UART0->UFSTAT>>8)&0x3f)
//										check_fifo_cnt = (UART0->UFSTAT>>8)&0x3f;
//						}
						write_tx_fifo(UART0_SERIAL);
				}
		}
}

static void write_tx_fifo(USERIAL serial)
{
		u8 buf[TX_FIFO_SIZE];
		u8 *buf_p = buf;
		u16 size;
		u32 int_index;

		size = TX_FIFO_SIZE - ((uart_register[serial]->UFSTAT>>8)&0x3f); //fifo中剩余空间大小
		if(((uart_register[serial]->UFSTAT) & (0x1<<14)))
				return;

		switch(serial) {
				case UART0_SERIAL: 
						int_index = INT_UART0;
						break;
				case UART1_SERIAL:
						int_index = INT_UART1;
						break;
				case UART2_SERIAL:
						int_index = INT_UART2;
						break;
				default:
						return;
		}

		disable_irq(int_index);
		size = read_cbuff(uart_hd[serial].write_cbuff, buf, size);
		while(size--) {
				uart_register[serial]->UTXH =  *buf_p++;
		}
		enable_irq(int_index);
}
/*----------------------------------------------------------------
**	函数	：uart_putbyte(u8 serial, u8 byte)
**	功能	：发送一个字节
**	参数	：serial：串口号	byte：要发送的字节
**	返回值：无
**	备注	：无
-------------------------------------------------------------------*/
void uart_putbyte(USERIAL serial, char byte)
{
		while(get_cbuff_remain(uart_hd[serial].write_cbuff) == 0)  { //cbuff已满，等待
				delay(10);	
		}


		u32 int_index;
		switch(serial) {
				case UART0_SERIAL: 
						int_index = INT_UART0;
						break;
				case UART1_SERIAL:
						int_index = INT_UART1;
						break;
				case UART2_SERIAL:
						int_index = INT_UART2;
						break;
				default:
						return;
		}

		disable_irq(int_index);
		write_cbuff(uart_hd[serial].write_cbuff,&byte, 1);
		enable_irq(int_index);
		if(check_cbuff_mutu(uart_hd[serial].write_cbuff) == 0){ //中断没有在使用cbuff
				write_tx_fifo(serial);
		}
}
/*----------------------------------------------------------------
**	函数	：uart_write(u8 serial, u8 *buff)
**	功能	：发送一个字符串
**	参数	：serial：串口号	buff：要发送的的字符串缓存
**	返回值：无
**	备注	：无
-------------------------------------------------------------------*/
void uart_write(USERIAL serial, char *buff, u8 size)
{
		u32 int_index;
		while(get_cbuff_remain(uart_hd[serial].write_cbuff) < size)  {
				delay(10);	
		}

		switch(serial) {
				case UART0_SERIAL: 
						int_index = INT_UART0;
						break;
				case UART1_SERIAL:
						int_index = INT_UART1;
						break;
				case UART2_SERIAL:
						int_index = INT_UART2;
						break;
				default:
						return;
		}

		disable_irq(int_index);
	  size = 	write_cbuff(uart_hd[serial].write_cbuff,buff, size);
		enable_irq(int_index);
		if(check_cbuff_mutu(uart_hd[serial].write_cbuff) == 0){ //中断没有在使用cbuff
				write_tx_fifo(serial);
		}
}


static void uart0_putbyte(char c)
{
		uart_putbyte(UART0_SERIAL, c);
}

void uart1_putbyte(char c)
{
		uart_putbyte(UART1_SERIAL, c);
}

void uart0_puts(void  *buff, u16 size)
{
		uart_write(UART0_SERIAL, buff, size);
}

void uart1_puts(void *buff, u16 size)
{
		uart_write(UART1_SERIAL, buff, size);
}
//从CBUFF读取一条字符串
u16 uart0_read_string(char *string, u16 len)
{
		if(uart_hd[UART0_SERIAL].package_cnt) {
				uart_hd[UART0_SERIAL].package_cnt--;
				return  read_cbuff_string(uart_hd[UART0_SERIAL].read_cbuff,string, len);
		}

		return 0;
}

u32 uart0_read(char *buffer, u32 size)
{
		return read_cbuff(uart_hd[UART0_SERIAL].read_cbuff, buffer, size);
}

