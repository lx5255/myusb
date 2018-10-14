#include "config.h"
#include "stdarg.h"
#include "task_manage.h"
#include "chain.h"
//#include "stdlib.h"

//#define TASK_MG_DEBUG
#ifdef TASK_MG_DEBUG
#define task_printf	uprintf
#else
#define task_printf(...)
#endif
static chain_point *task_chain = 0;
task_ls *cur_task = 0;


extern task_ls cmd_task_ls;
//extern task_ls usb_task_ls;

extern __task_list_start, __task_list_end;
u8 task_run()
{
		u8 res;
		chain_point *task_point;
#ifdef CMD_LINE
		res =  creat_task(&cmd_task_ls, 50);
	//	if(res)
	//			return 1;
#endif

		task_ls *task_ptr = &__task_list_start;
		for(; task_ptr < &__task_list_end; task_ptr++)
		{
				if(task_ptr->run_flag == 1)
				{
						task_printf("run %s\n", task_ptr->name);
						 creat_task(task_ptr, 50);	
				}
		}
		task_printf("aaa");

		while(1)
		{
				if(task_chain == 0)
				{
						return TM_TASK_NOEXIST;
				}
				task_point = task_chain;
//				task_printf("b");
				while(task_point)
				{
						cur_task = task_point->data;
						if(cur_task->task_func(0))
								return TM_RUN_ERR;  //���д���
								
//						task_printf("c");
						task_point = task_point->next;
				}
		}
}

//����һ������
u8 creat_task(task_ls *task_list, u16 msg_size)
{
    //task_ls *task_list = (task_ls *)malloc(sizeof(task_ls));
    if(task_list == NULL)
        return TM_PTR_NULL;
    
    if(task_list->task_init)  //��������ǰ��ʼ��
    {
        if(task_list->task_init())
            return TM_INIT_FAIL;//��ʼ��ʧ��
    }
    task_list->q_msg = (q_msg_struct *)new_q_msg(task_list->name, msg_size);
    if(task_list->q_msg == 0)
        return TM_MALLOC_ERR;
    
    add_chain_point(&task_chain, task_list);//������ҵ���������
		task_list->run_flag = 1;  
    return 0;
}
//ɾ��һ������
u8 del_task(char *name)
{
    task_ls *task_list;
    chain_point *chain_poin = task_chain;

    while(chain_poin)
    {
        task_list = task_chain->data;
        if(cmp_str(name, task_list->name))
        {
            if(task_list->task_exit)
                task_list->task_exit();
                
            del_chain_point(&task_chain, task_list);
            free_q_msg(task_list->q_msg);
         //   free(task_list);
            return 0;
        }
        chain_poin = chain_poin->next;
    }
    return TM_TASK_NOEXIST;   
}

//post��Ϣ���߳�
u8 taskq_post(char *task_name, u32 msg)
{
    task_ls *task_list;
    chain_point *chain_poin = task_chain;
		
    while(chain_poin)
    {
        task_list = chain_poin->data;
        if(cmp_str((const char *)task_name, (const char *)task_list->name))
        {
            u8 res;
            res =  q_post(task_list->q_msg, msg);
            if(res)
                return TM_OTHER;
            else
                return 0;
        }
        chain_poin = chain_poin->next;
    }
    
    return 1;
}

//��ȡһ����Ϣ
u32 taskq_pend(void)
{
    u32 msg;
    if(cur_task == 0)
        return 0xffffffff;

    msg = q_pend(cur_task->q_msg);
    
    return msg;
}

//����4����Ϣ
u8 task_post_msg(char *name, u8 len, ...)
{
		u16 msg_cnt;
		u32 msg[4] = {0,0,0,0};
		u8 cnt = 0, res;
		va_list ap;
		if(cur_task == 0)
		{
				return TM_TASK_NOEXIST;
		}
		msg_cnt = get_q_msg_free(cur_task->q_msg);
		if(msg_cnt < len)
		{
				return TM_OVER;
		}
		va_start(ap,len);

		while(cnt<len)
		{
				msg[cnt++] = va_arg(ap, int);
		}
		va_end(ap);

		for(cnt=0; cnt<4;)
		{
				res = taskq_post(name, msg[cnt++]);
				if(res)
						return TM_OTHER;
		}
		return 0;
}


//��ȡ4����Ϣ
u8 task_pend_msg(u32 *msg)
{
    u8 cnt;
    if(get_q_msg_num(cur_task->q_msg)<4)
    {
        msg[0] = 0xffffffff;
        return TM_NO_MSG;
    }
    for(cnt = 0; cnt<4;cnt++)
       msg[cnt] =  taskq_pend();

    return 0;
}

u16 get_task_msg_num()
{
    return cur_task->q_msg->msg_num;
}


#ifdef CMD_LINE
#include "cmd.h"

const char *task_cmd_table[]=
{
		"ps",  //��ӡ��ǰ���е�����
};

int task_cmd_deal(int cmd_index, char *arg)
{
		switch(cmd_index)
		{
				case 0:
						{
								task_ls *task_list;
								chain_point *chain_poin = task_chain;

								while(chain_poin) {
										task_list = chain_poin->data;
										uprintf("task:%s\n", task_list->name); 
										chain_poin = chain_poin->next;
								}

						}

						break;
				default:
						return 1;
		}
		return 0;
}
int task_do(char **arg, int arg_num)
{
		char task_cmd_cnt;; 
		while(arg_num--)		{
				for(task_cmd_cnt = 0; task_cmd_cnt<sizeof(task_cmd_table)/sizeof(task_cmd_table[0]); task_cmd_cnt++) {
						if(strcmp(*arg++,task_cmd_table[task_cmd_cnt]) == 0)	{
								task_cmd_deal(task_cmd_cnt, *arg);
						}
				}
		}
}
REGISTER_CMD(tk, task_do, "������������");
#endif
