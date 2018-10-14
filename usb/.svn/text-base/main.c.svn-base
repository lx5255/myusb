#include "config.h"
#include "task_manage.h"
//u8 malloc_buff[10000];
//extern __bss_start, __bss_end;

extern u32 check_fifo_cnt;
int main(void)
{
//	u8 *bss =& __bss_start;
//	for(; bss<&__bss_end;*bss++ = 0);
//	init_mem_alloc(malloc_buff, sizeof(malloc_buff));
  serial_init(115200);
  uprintf("hello word\n");	
	DIS_CPU_INT
//	uprintf("bss start :0x%x, end:0x%x\n\r", &__bss_start,& __bss_end);
//	printf_buff(&__bss_start, 100);	
//  EN_CPU_INT
//  enable_cpu_int();
/*
  int i = 0;
	for(; i<2048; i++){
//		putc('-'); 
//		putc('|');
		uprintf("-");
		uprintf("|");
	}
	*/
//	uprintf("check fifo %d\n\r", check_fifo_cnt);
	task_run();

	return 0;
}
