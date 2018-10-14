/***********************************************************************************************
 *
 *	日期: 2018/2/21  20:47
 *
 *	作者：liuxing
 *
 *  备注：公共命令函数
 *
 *
 * *******************************************************************************************/

#include "config.h"
#include "cmd_common.h"
#ifdef CMD_LINE
extern __cmd_list_start, __cmd_list_end;


u8 help_cmd(char **arg, int arg_num);
u8 reboot_cmd(char **arg, int arg_num);
u8 readreg_cmd(char **arg, int arg_num);
CMD_SECTION
const cmd_handle cmd_handler[] =
{
	{"help", help_cmd, "	打印帮助信息 {命令}"},
	{"reboot", reboot_cmd, "重启系统 {命令}"},
	{"readreg", readreg_cmd, "读取寄存器的值 {命令 [寄存器地址]}"},
};


/*----------------------------------------------------------------
**	函数	：u8 help_cmd(char **arg, int arg_num)
**	功能	：帮助命令处理函数
**	参数	：arg：参数缓存   arg_num：参数个数
**	返回值：0：成功  2：参数错误
**	备注	：
-------------------------------------------------------------------*/
u8 help_cmd(char **arg, int arg_num)
{	
		cmd_handle *cmd_table = &__cmd_list_start;
		u16 cmd_cnt;
		if(arg_num != 0)
		{
				return 2;
		}	
		for(cmd_cnt = 0; (void *)cmd_table<(void *)&__cmd_list_end;cmd_cnt++, cmd_table++)
		{
				cmd_printf("\n\r%d:%s	%s",cmd_cnt, cmd_table->cmd, cmd_table->cmd_help);
		}
		return 0;
}

u8 reboot_cmd(char **arg, int arg_num)
{	
		u8 i;
		void (*boot)(void) = (void *)0;

		if(arg_num != 0)
		{
				return 2;
		}	

		boot();
		return 0;
}

u8 readreg_cmd(char **arg, int arg_num)
{	
		u8 i;
		volatile u32 *reg =  (volatile u32 *)hex_to_int(arg[0]);

		if(arg_num != 1)
		{
				return 2;
		}	

		printf_u32((u32)reg);
		cmd_printf("reg vlue:");
		printf_u32(*reg);
		printf_bin(*reg);
		return 0;
}


#endif
