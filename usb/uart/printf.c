#include "config.h"
#include "s3c2440_uart.h"
#include "stdarg.h"
#include "s3c2440_printf.h"
/*
**	函数	：_printf(void *buff, ...)
**	功能	：通过串口1打印信息
**	参数	：buff:打印的数据缓存
**	返回值：无
**	备注	：无
*/
#if 0
void uart_printf(u8 serial, char *buff, ...)
{
	va_list ap;
	char write_buff[100];
	char *bufpointer = write_buff;
	
	va_start(ap,buff);
	vsprintf(write_buff, buff, ap);
	va_end(ap);
	while(*bufpointer)
	{
		uart_putbyte(serial,*bufpointer++);
	}
	//uart_write(serial, write_buff);
}
#endif
void uputs(void *buff, u16 len);
static uart_handle *uart_hand;
//打印初始化
inline u8 serial_init(u32 baud)
{
#ifdef DEBUG
		uart_hand = uart_open(PRINT_SERIAL, baud);
		if(uart_hand == NULL)
		{
				return 1;
		}
		else
		{
				return 0;
		}
#endif
}
void uprintf(char *buff, ...)
{
#ifdef DEBUG
		va_list ap;
		char write_buff[100];
		char *bufpointer = write_buff;

		va_start(ap,buff);
		vsprintf(write_buff, buff, ap);
		va_end(ap);
		uputs(write_buff, strlen(write_buff));	
#endif
}

char getc()
{
		return 0;
}
void putc(char byte)
{
#ifdef DEBUG
		uart_hand->io.putbyte(byte);	
#endif
}
void uputs(void *buff, u16 len)
{
#ifdef DEBUG
		uart_hand->io.puts(buff, len);	
#endif
}
void printf_u32(u32 value)
{
		u8 i, ch;

		uprintf( "0x");
		i = 8;
		do{
				i--;
				ch = (value>>i*4)&0xf;
				if(ch<10)
				{
//						putc('0'+ch);
				}
				else
				{
//						putc('a'+ch-10);
				}
		}while(i);
		putc(' ');
}

void printf_u8(u8 value)
{
		u8 i, ch;

		i = 2;
		uprintf( "0x");
		do{
				i--;
				ch = (value>>i*4)&0xf;
				if(ch<10)
				{
						putc('0'+ch);
				}
				else
				{
						putc('a'+ch-10);
				}
		}while(i);
		putc(' ');
}

void printf_u16(u16 value)
{
		u8 i, ch;
		i = 4;
		uprintf( "0x");
		do{
				i--;
				ch = (value>>i*4)&0xf;
				if(ch<10)
				{
						putc('0'+ch);
				}
				else
				{
						putc('a'+ch-10);
				}
		}while(i);
		putc(' ');
}

void printf_buff(const u8 *buff, u32 size)
{
		while(size--)
		{
				printf_u8(*buff++);
//				putc(' ');	
				//uart_write(PRINT_SERIAL, " ");
		}
}

void printf_bin(const u32 value)
{
		u8 cnt = 32;

		for(; cnt>0; cnt--)
		{
				uprintf("bit%d:", cnt-1);
				if(value & (0x01<<(cnt-1)))
				{
						uprintf("1 ");
				}
				else
				{
						uprintf("0 ");
				}
		}

}
