/***********************************************************************************************
 *
 *	����: 2018/2/21  20:47
 *
 *	���ߣ�liuxing
 *
 *  ��ע�������к��ĺ���
 *
 *
 * *******************************************************************************************/

#include "config.h"
#include "cmd.h"
#include "comm.h"
#include "task_manage.h"

#ifdef CMD_LINE
#ifndef MALLOC_EN
#define CMD_INPUT_MAX				5      // ��༸����������Դ
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
**	����	��������һ�������ȡ������
**	����	��input_hd_ret:��ȡ�����������Դ���
**	����ֵ��NULL 
**	��ע	��
-------------------------------------------------------------------*/
void copy_last_cmd(cmd_input_hd *input_handle)
{
		u16 cur_cmd_len = input_handle->data_end_ptr;  //�Ȱѵ�ǰ���ݳ��ȴ�����
		char *cmd_buffer = input_handle->cmd_buff;
		u16 cmd_parm_cnt = 0;
		char *input;

		input_handle->data_end_ptr = 0;
		input = cmd->cmd;
	//	strcpy(cmd_buffer, cmd->cmd); 
		for(;(*cmd_buffer++ = *input++) != '\0'; ); //��������
		cmd_buffer--;  //��ȥ'\0'
		for(; cmd_parm_cnt<cmd->cmdarg_num; cmd_parm_cnt++){
				*cmd_buffer++ = ' ';
				input = cmd->cmdarg[cmd_parm_cnt];
				for(;(*cmd_buffer++ = *input++) != '\0'; ); //��������
				cmd_buffer--;
		}
		input_handle->data_end_ptr = cmd_buffer - input_handle->cmd_buff;  //��ȡ����
		input_handle->curr_in_ptr = input_handle->data_end_ptr;

		putc('\r');  //���ԭ������ʾ
		while(cur_cmd_len--){
				putc(' ');
		}
		uprintf("\r$:");
		uprintf(input_handle->cmd_buff);
}
/*----------------------------------------------------------------
**	����	��������Դ�����ж�ȡ����
**	����	��input_hd_ret:��ȡ�����������Դ���
**	����ֵ��0:δ��������  1����ȡ������ 
**	��ע	��
-------------------------------------------------------------------*/
void input_hd_next_cmd(cmd_input_hd *input_handle)
{
		char *p_next_cmd_start;
		u32 cmd_len;

		//cmd_printf("next cmd\n\r");
		for(p_next_cmd_start = input_handle->cmd_buff; (*p_next_cmd_start != '\n')&&\
		(*p_next_cmd_start != '\r') && (*p_next_cmd_start); p_next_cmd_start++){ //�ҵ���ǰ����Ľ���λ��
				if(p_next_cmd_start == (input_handle->cmd_buff + input_handle->data_end_ptr)){  //��������ĩβ
						break;
				}
		}
		for(; (*p_next_cmd_start == '\n')||(*p_next_cmd_start == '\r') || (*p_next_cmd_start == 0); p_next_cmd_start++){ //�˵���һ������Ľ�����־
				if(p_next_cmd_start == (input_handle->cmd_buff + input_handle->data_end_ptr)){
						break;
				}
		}
		cmd_len = p_next_cmd_start - input_handle->cmd_buff;
		cmd_len = input_handle->data_end_ptr - cmd_len;  //ʣ�����ݳ���
		memcpy(input_handle->cmd_buff, p_next_cmd_start+1, cmd_len);
		input_handle->data_end_ptr = cmd_len;  //��ǰ�����ȡָ��
		input_handle->curr_in_ptr = cmd_len;
}
/*----------------------------------------------------------------
**	����	��������Դ�����ж�ȡ����
**	����	��input_hd_ret:��ȡ�����������Դ���
**	����ֵ��0:δ��������  1����ȡ������ 
**	��ע	��
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
				read_len = p_input->func(buffer,read_len);  //��ȡ�����ַ��� 

				if(read_len){
						char c;
						bool cmd_end_flag = false;
						//cmd_printf("len %d ", read_len);
						if(read_len == 3){
								if((buffer[0] == 0x1b)&&(buffer[1] == 0x5b)){  //���򰴼�����
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
						while(read_len--){  //����Ƿ������������־
								c = *p_buffer++;// p_input->cmd_buff[p_input->data_end_ptr++]; 
						//		cmd_printf("%x ", c);
								putc(c);  //��������
								if(c == '\b'){ // �˸�
									//	memcpy(&p_input->cmd_buff[p_input->data_end_ptr - 2],\
										&p_input->cmd_buff[p_input->data_end_ptr], read_len);  //��֮ǰһ���ַ�����,ɾ��
										p_input->data_end_ptr --; //�˸�
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
								return true;  //����ȡ�������������������Դ�������
						}
				}
				p_input = p_input->next;  //��һ����������Դ
		}
		return false;
}
/*----------------------------------------------------------------
**	����	������һ��������
**	����	��cmd�����������  input:��ȡ�����������Դ
**	����ֵ����������� 
**	��ע	��
-------------------------------------------------------------------*/
u32 cmd_string_ansys(cmd_list *cmd, cmd_input_hd *input)
{
		char *p_out, *p_in;

		p_out = cmd->cmd;
		p_in = input->cmd_buff;

		cmd_printf("cmd ansys start buff ptr %d\n\r", input->data_end_ptr);
		while(*p_in == ' ') { // �˵�ǰ����ֵĿո�
				p_in++;
		}
#ifdef MALLOC_EN
		while(cmd->cmdarg_num--){  //�ͷŲ���������
				free(cmd->cmdarg[cmd->cmdarg_num]);
		}
#endif
		cmd->cmdarg_num = 0;
//		cmd_printf("1\n\r");
		/*-----------------------------ȡ������----------------------------*/
		while(*p_in != ' '){  //��ǰ�ַ����ǿո�
		//		cmd_printf("2\n\r");
				if((*p_in == '\r')||(*p_in == '\n')||(*p_in == 0)){  //�س�������� �������
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

		/*--------------------------ȡ������--------------------------------*/
		while((*p_in != '\r')&&(*p_in != '\n')&&(*p_in != 0)) {
				if(*p_in == ' '){ //�пո������ǰ��������
						*p_out = 0; //Ϊ�ϸ��������ӽ�����
						while(*p_in == ' '){ //��ȥ�ո�
								p_in++;
						} 
						if((*p_in == '\r')||(*p_in == '\n') || (*p_in == 0)){  //ȷ���ո���ǽ���
//								cmd_printf("4");
								break;
						}
						cmd->cmdarg_num++;
						if(cmd->cmdarg_num == CMD_PARM_MAX){  //���������������ֵ
								return 1;
						}
#ifdef MALLOC_EN
						cmd->cmdarg[cmd->cmdarg_num - 1] =(char *)malloc(CMD_PARM_LEN);
#else
#endif
						p_out = cmd->cmdarg[cmd->cmdarg_num - 1];  //���������������
				} else {
						*p_out++ = *p_in++;
						if(p_out - cmd->cmdarg[cmd->cmdarg_num - 1] >= CMD_PARM_LEN - 1){  //�����˲�������󳤶�
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
**	����	����������
**	����	��cmd�����������
**	����ֵ����������� 
**	��ע	��
-------------------------------------------------------------------*/
u32 cmd_deal(cmd_list *cmd)
{
		u8 res = 0;
		cmd_handle *cmd_table = (cmd_handle *)&__cmd_list_start;	

		//cmd_printf("cmd deal %s   num %d %s %s\n\r", cmd->cmd, cmd->cmdarg_num, cmd->cmdarg[0], cmd->cmdarg[1]);
		for(; (void *)cmd_table<(void *)&__cmd_list_end; cmd_table++) {
				if(cmdcmp(cmd->cmd, cmd_table->cmd) == 0) {
						cmd_printf("cmd:%s/%s\n\r", cmd->cmd, cmd_table->cmd);
						res = cmd_table->cmd_function(cmd->cmdarg, cmd->cmdarg_num); //�������������
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
**	����	��u8 read_cmd(cmd_list *cmd)
**	����	����ȡһ������
**	����	��cmd�����������
**	����ֵ�������: 0���ɹ�  1���ڴ����ʧ�� 2��δ��ȡ������ 
						 3���ϴ����δ����      4:����Խ��
**	��ע	��
-------------------------------------------------------------------*/
u8 read_cmd(cmd_list *cmd)
{
	char cmd_buff[80];
	char *p_in = 0, *p_out;
	
	if(cmd->cmd){  //��һ�ε����δ����
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
		/*-----------------------------ȡ������----------------------------*/
		while(*p_out != ' '){  //��ǰ�ַ����ǿո�
			if((*p_out == '\r')||(*p_out == '\n')||(*p_out == 0)){  //�س�������� �������
				*p_in = 0;
				cmd->cmdarg_num = 0;
				return 0;
			}
			*p_in++ = *p_out++;
			if(p_in >= (cmd->cmd + 30)) {
				return 4;
			}
		}
		
		cmd->cmdarg = (char **)malloc(sizeof(char **)*20); //�����ڴ�
		if(cmd->cmdarg == 0) {
			return 1;
		}
		/*--------------------------ȡ������--------------------------------*/
		while((*p_out != '\r')&&(*p_out != '\n')&&(*p_out != 0)) {
			if(*p_out == ' '){ //�пո������ǰ��������
				*p_in = 0; //Ϊ�ϸ��������ӽ�����
				while(*p_out == ' '){ //��ȥ�ո�
					p_out++;
				} 
				cmd->cmdarg_num++;
				
				cmd->cmdarg[cmd->cmdarg_num - 1] = (char *)malloc(50);  //Ϊ������������ڴ�			
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
**	����	��u8 cmd_ansys(cmd_list *cmd)
**	����	����������
**	����	��cmd�����������
**	����ֵ�����������
**	��ע	��
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
						res = cmd_table->cmd_function(cmd->cmdarg, cmd->cmdarg_num); //�������������
						if(0 == res) {
								cmd_printf("run cmd success\n\r");
								break;
						} else {
								cmd_printf("run cmd err:%d\n\r", res);
						}	
				}
		}
		/*-------------����������ͷ�-----------------*/
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
**	����	��u8 cmdcmp(const char *str1, const char *str2)
**	����	������Ƚ�
**	����	��str1�������ַ���1  str2�������ַ���2
**	����ֵ��0��������ͬ  ��������ͬ��ַƫ��
**	��ע	��
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
		register_cmd_in_func(uart_hd->io.read); //ע�ᴮ��Ϊ�������뷽ʽ	
}

void cmd_line_exit()
{
		cmd_input_hd *input_hd_link, *free_link;		
		input_hd_link = input_hd; 
#ifdef MALLOC_EN
		while(input_hd_link) { // �ͷ�����
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
		for(table_index = 0; table_index<sizeof(read_func_table)/sizeof(cmd_input_hd); table_index++){ //�������ȡһ���յĴ�����
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
				while(link_tail->next){  //Ѱ��������β
						link_tail = link_tail->next;	
				} 
				link_tail->next = new_link;  //���½ڵ���ص�����ĩβ
		}
		return true;
}




/*----------------------------------------------------------------------------------------
 *
 *													cmd������
 *
 *----------------------------------------------------------------------------------------*/

//cmd����
u8 cmd_task(void *p)
{
		cmd_input_hd *cmd_line_input = NULL;
    //read_cmd(&cmd);
   // cmd_ansys(&cmd);
		if(read_cmd(&cmd_line_input) == true){ //��ȡ��һ������
		  u32 ret;
			uprintf("\n\r");
			ret = cmd_string_ansys(cmd, cmd_line_input); // �����ַ��� 
			input_hd_next_cmd(cmd_line_input);  //���ǰһ������Ļ���
			if(ret == 0){
				cmd_deal(cmd); //ִ������	
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