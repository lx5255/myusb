#include "config.h"
extern __bss_start, __bss_end;
void clean_bss(void)
{
		u8 *bss =& __bss_start;
		for(; bss<&__bss_end;*bss++ = 0);
}

static u8 mem_buff[MEM_MAX];
void sys_init(void)
{
		clean_bss();
		init_mem_alloc(mem_buff, sizeof(mem_buff));
}


