#ifndef _PRINTF
#define _PRINTF
#include "s3c2440_uart.h"

//#define uprintf(a,...)  uart_printf(PRINT_SERIAL, a,...)
void uart_printf(u8 serial, char *buff, ...);
void printf_u32(u32 value);
void printf_u8(u8 value);
void printf_u16(u16 value);
void printf_buff(const u8 *buff, u32 size);
void uprintf(char *buff, ...);
void printf_bin(const u32 value);
#endif
