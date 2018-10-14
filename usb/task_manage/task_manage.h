#ifndef _TASK_MANAGE_
#define _TASK_MANAGE_
#include "q_msg.h"
//#include "msg.h"

typedef struct
{
    char *name;
    u8 (*task_func)(void *arg);
    u8 (*task_init)(void);
    u8 (*task_exit)(void);
    q_msg_struct *q_msg;
		u8 run_flag;
}task_ls;

enum
{
		TM_TASK_EXIST = 0x01,
		TM_TASK_NOEXIST,
		TM_INIT_FAIL,
		TM_PTR_NULL,
		TM_MALLOC_ERR,
		TM_OVER,
		TM_NO_MSG,
		TM_RUN_ERR,
		TM_OTHER,
}task_err;
u8 task_run();
u8 creat_task(task_ls *task_list, u16 msg_size);
u8 del_task(char *name);
u8 taskq_post(char *task_name, u32 msg);
u32 taskq_pend(void);
u8 task_post_msg(char *name, u8 len, ...);
u8 task_pend_msg(u32 *msg);
u16 get_task_msg_num();

#define register_task_list __attribute__((section(".task_list"))) task_ls  

#endif
