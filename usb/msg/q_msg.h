#ifndef _Q_MSG_
#define _Q_MSG_
#include "config.h"

typedef struct 
{
    u32 *msg_buff;
    u16 buff_size;
    u16 msg_num;
    u16 r_ptr;
    u16 w_ptr;
    char *name;
}q_msg_struct;

q_msg_struct *new_q_msg(char *name, u16 msg_size);
void free_q_msg(q_msg_struct *q_msg);
u8 q_post(q_msg_struct *q_msg, u32 msg);
u32 q_pend(q_msg_struct *q_msg);
u16 get_q_msg_num(q_msg_struct *q_msg);
void q_msg_flush(q_msg_struct *q_msg);
u16 get_q_msg_free(q_msg_struct *q_msg);


#endif
