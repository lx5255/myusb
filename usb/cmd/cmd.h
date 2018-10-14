#ifndef _CMD_
#define _CMD_
#define CMD_SECTION	  __attribute__((section(".cmd_list")))

#define CMD_DEBUG
#ifdef CMD_DEBUG
#define cmd_printf  uprintf
#else
#define cmd_printf(...)
#endif

#define CMD_BUFF_LEN		128	//
#define CMD_LEN					32  //命令长度
#define CMD_PARM_LEN		64	//参数缓存长度
#define CMD_PARM_MAX		5		//参数最大个数
typedef struct cmd_list  
{
	char cmd[CMD_LEN];
	char *cmdarg[CMD_PARM_MAX];
	u8 cmdarg_num;
}cmd_list;

typedef struct cmd_handle  
{
	char *cmd;
	u8 (*cmd_function)(char **, int);
	char *cmd_help;
}cmd_handle;

typedef struct _cmd_input_hd
{
		u32 (*func)(char *, u32);
		struct _cmd_input_hd *next;
		char cmd_buff[CMD_BUFF_LEN];
//		cmd_list *cmd;
		u16 data_end_ptr;
		u16 curr_in_ptr;
}cmd_input_hd;

u8 cmdcmp(const char *str1, const char *str2);
void cmd_line_init();
void cmd_line_exit();
bool register_cmd_in_func(u32 (*input_func)(char *, u32));
void input_hd_next_cmd(cmd_input_hd *input_handle);
bool read_cmd(cmd_input_hd **input_hd_ret);
u32 cmd_string_ansys(cmd_list *cmd, cmd_input_hd *input);
u32 cmd_deal(cmd_list *cmd);

#define REGISTER_CMD(name, func, info) cmd_handle cmd_##name  CMD_SECTION\
= {#name, func, info}
#endif

