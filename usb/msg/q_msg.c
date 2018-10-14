#include "q_msg.h"
//#include "stdlib.h"
q_msg_struct *new_q_msg(char *name, u16 msg_size)
{
    q_msg_struct *new_q = (q_msg_struct *)malloc(sizeof(q_msg_struct));
    if(new_q == 0)
    {
        return 0;
    }

    new_q->msg_buff = (u32 *)malloc(sizeof(u32)*msg_size);
    new_q->name = name;
    new_q->msg_num = 0;
    new_q->buff_size = msg_size;
    new_q->w_ptr = 0;
    new_q->r_ptr = 0;
    return new_q;
}

void free_q_msg(q_msg_struct *q_msg)
{
    if(q_msg == 0)
    {
        return;
    }

    if(q_msg->msg_buff)
    {
        free(q_msg->msg_buff);
    }

    free(q_msg);
}


u8 q_post(q_msg_struct *q_msg, u32 msg)
{
    if(q_msg == 0)
    {
        return 1;
    }

    if(q_msg->msg_num == q_msg->buff_size) //没有空间
    {
        return 2;
    }

    q_msg->msg_buff[q_msg->w_ptr++] = msg;
    q_msg->msg_num++;
  //  uprintf("pt %d " , msg);
    if(q_msg->w_ptr == q_msg->buff_size) //到达边界
    {
        q_msg->w_ptr = 0;
    }
    return 0;
}

u32 q_pend(q_msg_struct *q_msg)
{
    u32 msg;
    if(q_msg == 0)
    {
        return 0xffffffff;
    }

    if(q_msg->msg_num == 0)//没有消息
    {
        return 0xffffffff;
    }

    msg = q_msg->msg_buff[q_msg->r_ptr++];
    q_msg->msg_num--;
  //  uprintf("pd %d ", msg);
    if(q_msg->r_ptr == q_msg->buff_size) //到达边界
    {
        q_msg->r_ptr = 0;
    }
   
    return msg;
}

u16 get_q_msg_num(q_msg_struct *q_msg)
{
     if(q_msg == 0)
    {
        return 0;
    }
    return q_msg->msg_num;
}

u16 get_q_msg_free(q_msg_struct *q_msg)
{
     if(q_msg == 0)
    {
        return 0;
    }
    return q_msg->buff_size- q_msg->msg_num;
}

void q_msg_flush(q_msg_struct *q_msg)
{
    q_msg->msg_num = 0;
    q_msg->w_ptr = 0;
    q_msg->r_ptr = 0;
}

