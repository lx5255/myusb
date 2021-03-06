/***********************************************************************************************
 *
 *	日期: 2018/2/21  20:47
 *
 *	作者：liuxing
 *
 *  备注：命令行核心函数
 *
 *
 * *******************************************************************************************/

#include "config.h"
#include "cmd.h"
#include "comm.h"
#include "task_manage.h"

#ifdef CMD_LINE
#ifndef MALLOC_EN
#define CMD_INPUT_MAX				5      // 最多几个命令输入源
static cmd_input_hd cmd_input_table[CMD_INPUT_MAX];  
static u8 read_func_use_flag = 0;
static cmd_list g_cmd;
static cmd_arg_buff[CMD_PARM_MAX][CMD_PARM_LEN];
#endif

extern __cmd_list_start, __cmd_list_end;
static uart_handle *uart_hd = NULL;
static cmd_input_hd *input_hd = NULL;
static cmd_list *cmd = NULL;
static void cmd_var_init();

/*----------------------------------------------------------------
**	功能	：复制上一条命令到读取缓存中
**	参数	：input_hd_ret:读取到命令的输入源句柄
**	返回值：NULL 
**	备注	：
-------------------------------------------------------------------*/
void copy_last_cmd(cmd_input_hd *input_handle)
{
		u16 cur_cmd_len = input_handle->data_end_ptr;  //先把当前数据长度存下来
		char *cmd_buffer = input_handle->cmd_buff;
		u16 cmd_parm_cnt = 0;
		char *input;

		input_handle->data_end_ptr = 0;
		input = cmd->cmd;
	//	strcpy(cmd_buffer, cmd->cmd); 
		for(;(*cmd_buffer++ = *input++) != '\0'; ); //拷贝数据
		cmd_buffer--;  //减去'\0'
		for(; cmd_parm_cnt<cmd->cmdarg_num; cmd_parm_cnt++){
				*cmd_buffer++ = ' ';
				input = cmd->cmdarg[cmd_parm_cnt];
				for(;(*cmd_buffer++ = *input++) != '\0'; ); //拷贝数据
				cmd_buffer--;
		}
		input_handle->data_end_ptr = cmd_buffer - input_handle->cmd_buff;  //获取长度
		input_handle->curr_in_ptr = input_handle->data_end_ptr;

		putc('\r');  //清除原来的显示
		while(cur_cmd_len--){
				putc(' ');
		}
		uprintf("\r$:");
		uprintf(input_handle->cmd_buff);
}
/*----------------------------------------------------------------
**	功能	：从输入源链表中读取命令
**	参数	：input_hd_ret:读取到命令的输入源句柄
**	返回值：0:未读到命令  1：读取到命令 
**	备注	：
-------------------------------------------------------------------*/
void input_hd_next_cmd(cmd_input_hd *input_handle)
{
		char *p_next_cmd_start;
		u32 cmd_len;

		//cmd_printf("next cmd\n\r");
		for(p_next_cmd_start = input_handle->cmd_buff; (*p_next_cmd_start != '\n')&&\
		(*p_next_cmd_start != '\r') && (*p_next_cmd_start); p_next_cmd_start++){ //找到当前命令的结束位置
				if(p_next_cmd_start == (input_handle->cmd_buff + input_handle->data_end_ptr)){  //到达数据末尾
						break;
				}
		}
		for(; (*p_next_cmd_start == '\n')||(*p_next_cmd_start == '\r') || (*p_next_cmd_start == 0); p_next_cmd_start++){ //滤掉上一个命令的结束标志
				if(p_next_cmd_start == (input_handle->cmd_buff + input_handle->data_end_ptr)){
						break;
				}
		}
		cmd_len = p_next_cmd_start - input_handle->cmd_buff;
		cmd_len = input_handle->data_end_ptr - cmd_len;  //剩余数据长度
		memcpy(input_handle->cmd_buff, p_next_cmd_start+1, cmd_len);
		input_handle->data_end_ptr = cmd_len;  //当前命令读取指针
		input_handle->curr_in_ptr = cmd_len;
}
/*----------------------------------------------------------------
**	功能	：从输入源链表中读取命令
**	参数	：input_hd_ret:读取到命令的输入源句柄
**	返回值：0:未读到命令  1：读取到命令 
**	备注	：
-------------------------------------------------------------------*/
bool read_cmd(cmd_input_hd **input_hd_ret)
{
		char *cmd_buffer;
		u32 read_len;
		char buffer[64];
		char *p_buffer;
		cmd_input_hd *p_input = input_hd;

		while(p_input){  
				cmd_buffer = &p_input->cmd_buff[p_input->data_end_ptr];
				read_len = CMD_BUFF_LEN - p_input->data_end_ptr;
				read_len = read_len > 64?64:read_len;
				read_len = p_input->func(buffer,read_len);  //读取命令字符串 

				if(read_len){
						char c;
						bool cmd_end_flag = false;
						//cmd_printf("len %d ", read_len);
						if(read_len == 3){
								if((buffer[0] == 0x1b)&&(buffer[1] == 0x5b)){  //方向按键处理
										if(buffer[2] == 0x41){
												copy_last_cmd(p_input);
												return false; 
										}else if(buffer[2] == 0x43){
												p_input->curr_in_ptr--;	
										//	putc(); putc(); putc();
												return false;
										}else if(buffer[2] == 0x44){
												if(p_input->curr_in_ptr < p_input->data_end_ptr){
														p_input->curr_in_ptr++;
														putc(0x1b); putc(0x5b); putc(0x44);
												}else{
														putc(0x1b); putc(0x5b); putc(0x43);
												}
												return false;
										}

								}
						}

						p_buffer = buffer;
						while(read_len--){  //检测是否有命令结束标志
								c = *p_buffer++;// p_input->cmd_buff[p_input->data_end_ptr++]; 
						//		cmd_printf("%x ", c);
								putc(c);  //回显数据
								if(c == '\b'){ // 退格
									//	memcpy(&p_input->cmd_buff[p_input->data_end_ptr - 2],\
										&p_input->cmd_buff[p_input->data_end_ptr], read_len);  //将之前一个字符覆盖,删除
										p_input->data_end_ptr --; //退格
										putc(' '); 
										putc('\b'); 
										continue;
								}
								if((c == '\n')||(c == '\r')||(p_input->data_end_ptr ==(CMD_BUFF_LEN - 1))){
				//						cmd_printf("cmd end\n\r");
										cmd_end_flag = true;
								}
								p_input->cmd_buff[p_input->data_end_ptr++] = c;
						}
						if(cmd_end_flag == true){
								*input_hd_ret = p_input; 
								return true;  //将读取到完整命令的命令输入源句柄返回
						}
				}
				p_input = p_input->next;  //下一个命令输入源
		}
		return false;
}
/*----------------------------------------------------------------
**	功能	：解析一条命令行
**	参数	：cmd：命令参数表  input:读取到命令的输入源
**	返回值：处理错误号 
**	备注	：
-------------------------------------------------------------------*/
u32 cmd_string_ansys(cmd_list *cmd, cmd_input_hd *input)
{
		char *p_out, *p_in;

		p_out = cmd->cmd;
		p_in = input->cmd_buff;

		cmd_printf("cmd ansys start buff ptr %d\n\r", input->data_end_ptr);
		while(*p_in == ' ') { // 滤掉前面出现的空格
				p_in++;
		}
#ifdef MALLOC_EN
		while(cmd->cmdarg_num--){  //释放参数缓存区
				free(cmd->cmdarg[cmd->cmdarg_num]);
		}
#endif
		cmd->cmdarg_num = 0;
//		cmd_printf("1\n\r");
		/*-----------------------------取出命令----------------------------*/
		while(*p_in != ' '){  //当前字符不是空格
		//		cmd_printf("2\n\r");
				if((*p_in == '\r')||(*p_in == '\n')||(*p_in == 0)){  //回车或结束符 命令结束
						*p_out = 0;
						cmd->cmdarg_num = 0;
						cmd_printf("cmd ansys end\n\r");
						return 0;
				}
				*p_out++ = *p_in++;
				if(((u32)p_out >= ((u32)cmd->cmd + CMD_LEN - 1))||((u32)p_in >= ((u32)input->cmd_buff + input->data_end_ptr))) {
				//		cmd_printf("3 out %x  cmd:%x   in:%x  buff:%x\n\r", p_out, (cmd->cmd + CMD_LEN - 1), p_in, (input->cmd_buff + input->data_end_ptr));
						return 2;
				}
		}

		/*--------------------------取出参数--------------------------------*/
		while((*p_in != '\r')&&(*p_in != '\n')&&(*p_in != 0)) {
				if(*p_in == ' '){ //有空格代表当前参数结束
						*p_out = 0; //为上个参数添加结束符
						while(*p_in == ' '){ //除去空格
								p_in++;
						} 
						if((*p_in == '\r')||(*p_in == '\n') || (*p_in == 0)){  //确保空格后不是结束
//								cmd_printf("4");
								break;
						}
						cmd->cmdarg_num++;
						if(cmd->cmdarg_num == CMD_PARM_MAX){  //参数数量超出最大值
								return 1;
						}
#ifdef MALLOC_EN
						cmd->cmdarg[cmd->cmdarg_num - 1] =(char *)malloc(CMD_PARM_LEN);
#else
#endif
						p_out = cmd->cmdarg[cmd->cmdarg_num - 1];  //输出到参数缓存区
				} else {
						*p_out++ = *p_in++;
						if(p_out - cmd->cmdarg[cmd->cmdarg_num - 1] >= CMD_PARM_LEN - 1){  //超出了参数的最大长度
								return 2;
						}
				}
				if(p_in >= (input->cmd_buff + input->data_end_ptr)){
						return 2;
				}
		}
		*p_out = 0;
		return 0;
}
/*----------------------------------------------------------------
**	功能	：处理命令
**	参数	：cmd：命令参数表
**	返回值：处理错误号 
**	备注	：
-------------------------------------------------------------------*/
u32 cmd_deal(cmd_list *cmd)
{
		u8 res = 0;
		cmd_handle *cmd_table = (cmd_handle *)&__cmd_list_start;	

		//cmd_printf("cmd deal %s   num %d %s %s\n\r", cmd->cmd, cmd->cmdarg_num, cmd->cmdarg[0], cmd->cmdarg[1]);
		for(; (void *)cmd_table<(void *)&__cmd_list_end; cmd_table++) {
				if(cmdcmp(cmd->cmd, cmd_table->cmd) == 0) {
						cmd_printf("cmd:%s/%s\n\r", cmd->cmd, cmd_table->cmd);
						res = cmd_table->cmd_function(cmd->cmdarg, cmd->cmdarg_num); //调用命令处理函数
						if(0 == res) {
								cmd_printf("run cmd success\n\r");
								break;
						} else {
								cmd_printf("run cmd err:%d\n\r", res);
						}	
				}
		}
		return res;
}

#if 0 /*----------------------------------------------------------------
**	函数	：u8 read_cmd(cmd_list *cmd)
**	功能	：读取一条命令
**	参数	：cmd：命令参数表
**	返回值：错误号: 0：成功  1：内存分配失败 2：未读取到命令 
						 3：上次命令还未处理      4:访问越界
**	备注	：
-------------------------------------------------------------------*/
u8 read_cmd(cmd_list *cmd)
{
	char cmd_buff[80];
	char *p_in = 0, *p_out;
	
	if(cmd->cmd){  //上一次的命令还未处理
		return 3; 
	}
	if(uart_hd->io.read(cmd_buff, sizeof(cmd_buff))) {
cmd_handle:
		cmd->cmd = (char *)malloc(30);
		if(!cmd->cmd) {
			return 1;
		}
		cmd_printf("cmd buff:%s\n\r", cmd_buff);
		p_in = cmd->cmd;
		p_out = cmd_buff;
		/*-----------------------------取出命令----------------------------*/
		while(*p_out != ' '){  //当前字符不是空格
			if((*p_out == '\r')||(*p_out == '\n')||(*p_out == 0)){  //回车或结束符 命令结束
				*p_in = 0;
				cmd->cmdarg_num = 0;
				return 0;
			}
			*p_in++ = *p_out++;
			if(p_in >= (cmd->cmd + 30)) {
				return 4;
			}
		}
		
		cmd->cmdarg = (char **)malloc(sizeof(char **)*20); //分配内存
		if(cmd->cmdarg == 0) {
			return 1;
		}
		/*--------------------------取出参数--------------------------------*/
		while((*p_out != '\r')&&(*p_out != '\n')&&(*p_out != 0)) {
			if(*p_out == ' '){ //有空格代表当前参数结束
				*p_in = 0; //为上个参数添加结束符
				while(*p_out == ' '){ //除去空格
					p_out++;
				} 
				cmd->cmdarg_num++;
				
				cmd->cmdarg[cmd->cmdarg_num - 1] = (char *)malloc(50);  //为参数缓存分配内存			
				if(cmd->cmdarg[cmd->cmdarg_num - 1] == 0) {
					return 1;
				}
				p_in = cmd->cmdarg[cmd->cmdarg_num - 1];
			} else {
				*p_in++ = *p_out++;
			}
		}
		*p_in = 0;
		return 0;
	} else {
		return 2;
	}
}

/*----------------------------------------------------------------
**	函数	：u8 cmd_ansys(cmd_list *cmd)
**	功能	：解析命令
**	参数	：cmd：命令参数表
**	返回值：命令处理错误
**	备注	：
-------------------------------------------------------------------*/
u8 cmd_ansys(cmd_list *cmd)
{
		u8 i;
		u8 res = 0;
		cmd_handle *cmd_table = (cmd_handle *)&__cmd_list_start;	

		if(cmd->cmd == 0) {
				return 1;
		}
		for(; (void *)cmd_table<(void *)&__cmd_list_end; cmd_table++) {
				if(cmdcmp(cmd->cmd, cmd_table->cmd) == 0) {
						cmd_printf("cmd:%s/%s\n\r", cmd->cmd, cmd_table->cmd);
						res = cmd_table->cmd_function(cmd->cmdarg, cmd->cmdarg_num); //调用命令处理函数
						if(0 == res) {
								cmd_printf("run cmd success\n\r");
								break;
						} else {
								cmd_printf("run cmd err:%d\n\r", res);
						}	
				}
		}
		/*-------------将命令缓存区释放-----------------*/
		while(cmd->cmdarg_num--) {
				free(cmd->cmdarg[cmd->cmdarg_num]);
		}
		free(cmd->cmdarg);
		cmd->cmdarg = 0;
		free(cmd->cmd);
		cmd->cmd =0;
		cmd->cmdarg_num = 0;
		return res;
}
#endif
/*----------------------------------------------------------------
**	函数	：u8 cmdcmp(const char *str1, const char *str2)
**	功能	：命令比较
**	参数	：str1：命令字符串1  str2：命令字符串2
**	返回值：0：命令相同  其他：不同地址偏移
**	备注	：
-------------------------------------------------------------------*/
u8 cmdcmp(const char *str1, const char *str2)
{
	while(1) {
		if(*str1 != *str2) {
				return 1;
		} else {
				if(*str1 == '\0') {
						return 0;
				}
				str1++;
				str2++;
		}
	}
}
static void cmd_var_init()
{
#ifdef MALLOC_EN
		 cmd = (cmd_list *)malloc(sizeof(cmd_list));
		 if(cmd == NULL){
				uprintf("malloc err\n\r");
		 }
#else
		u8 argcnt = 0;
		cmd = &g_cmd;
	 for(; argcnt<CMD_PARM_MAX; argcnt++){
				cmd->cmdarg[argcnt] = cmd_arg_buff[argcnt];
	 }
#endif
		cmd->cmdarg_num = 0;
}

void cmd_line_init()
{
		cmd_var_init();
		uart_hd = get_uart_hd(UART0_SERIAL);
		if(uart_hd == NULL)
		{
				return ;
		}
		register_cmd_in_func(uart_hd->io.read); //注册串口为命令输入方式	
}

void cmd_line_exit()
{
		cmd_input_hd *input_hd_link, *free_link;		
		input_hd_link = input_hd; 
#ifdef MALLOC_EN
		while(input_hd_link) { // 释放链表
				free_link = input_hd_link;
				input_hd_link = input_hd_link->next;
	//			free(free_link->cmd);
				free(free_link);
		}
#else
		read_func_use_flag = 0;
#endif
		input_hd = NULL;
}

bool register_cmd_in_func(u32 (*input_func)(char *, u32))
{
		cmd_input_hd *new_link = 0;
#ifdef MALLOC_EN
		new_link = (cmd_input_hd *)malloc(sizeof(cmd_input_hd));
		if(new_link == NULL){
				cmd_printf("malloc err");
				return false;
		}
/*		new_link->cmd = (cmd_list *)malloc(sizeof(cmd_list));
		if(new_link->cmd == NULL){
				free(cmd);
				cmd_printf("malloc err");
				return false;
		}
*/	
#else
		u8  table_index;
		for(table_index = 0; table_index<sizeof(read_func_table)/sizeof(cmd_input_hd); table_index++){ //从数组获取一个空的储存区
				if(read_func_use_flag & BIT(table_index) == 0){
						new_link = &read_func_table[table_index];
//						new_link->cmd = &g_cmd[table_index];
						read_func_use_flag |= BIT(table_index);
				}
		}
		if(new_link == 0){
				cmd_printf("max read func\n\r");
				return false;
		}
#endif
		new_link->func = input_func;
		new_link->next = NULL;
		new_link->data_end_ptr = 0;
		new_link->curr_in_ptr = 0;
		if(input_hd == NULL){
				input_hd = new_link;	
		}else{
				cmd_input_hd *link_tail;
				link_tail = input_hd;
				while(link_tail->next){  //寻找链表结尾
						link_tail = link_tail->next;	
				} 
				link_tail->next = new_link;  //将新节点挂载到链表末尾
		}
		return true;
}




/*----------------------------------------------------------------------------------------
 *
 *													cmd任务函数
 *
 *----------------------------------------------------------------------------------------*/

//cmd任务
u8 cmd_task(void *p)
{
		cmd_input_hd *cmd_line_input = NULL;
    //read_cmd(&cmd);
   // cmd_ansys(&cmd);
		if(read_cmd(&cmd_line_input) == true){ //读取到一条命令
		  u32 ret;
			uprintf("\n\r");
			ret = cmd_string_ansys(cmd, cmd_line_input); // 解析字符串 
			input_hd_next_cmd(cmd_line_input);  //清除前一个命令的缓存
			if(ret == 0){
				cmd_deal(cmd); //执行命令	
			}else{
				cmd_printf("ansys cmd string err");
			}
			uprintf("\n\r$:");
		}
    return 0;
}

u8 cmd_task_init()
{		
		cmd_line_init();
		return 0;
}

u8 cmd_task_exit()
{ 		
		cmd_line_exit();
}
const task_ls cmd_task_ls =
{
    .name = "cmd_task",
    .task_func = cmd_task,
    .task_init = cmd_task_init,
    .task_exit = cmd_task_exit,
	  .q_msg = 0,
};
#endif
