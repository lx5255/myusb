#ifndef _CONFIG_
#define _CONFIG_
#include "comm.h"
#include "s3c2440_printf.h"
#include "my_malloc.h"
#include "irq.h"
#define MALLOC_EN								   //是否使用内存分配	
#define PRINT_SERIAL UART0_SERIAL  //使用哪个串口做打印口
#define DEBUG                      //是否开打印
#define BOOT_START							   //是否是uboot启动
#define CMD_LINE								   //是否使用命令行


#ifdef MALLOC_EN
#define _MY_MALLOC 1   //使用自己写的内存分配

#if _MY_MALLOC
#define malloc  my_malloc
#define free my_free
#define MEM_MAX							(0x20000)
#endif
#endif

#define NULL		(void *)0
#define TRUE		1
#define FALSE		0
#define true		1
#define false		0

#endif

