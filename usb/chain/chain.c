#include "config.h"
#include "chain.h"

//添加一个链表的节点，如果链表头为空，自动创建链表
u8 add_chain_point(chain_point **chain_head, void *data)
{
    chain_point *new_point = (chain_point *)malloc(sizeof(chain_point ));
    if(new_point == 0)
    {
        return 1;
    }
    new_point->data = data;
    
    if(*chain_head == 0)  //当前链表没有节点 第一个节点
    {
        new_point->prev = 0;
        new_point->next = 0;
        *chain_head = new_point;
    }
    else
    {
        while((*chain_head)->next) *chain_head = (*chain_head)->next;  //找到最后一个节点
        new_point->prev = *chain_head;  //将新节点挂到链表尾部
        (*chain_head)->next = new_point;
    }
    return 0;
}

//从链表中删除一个节点
u8 del_chain_point(chain_point **chain_head, void *data)
{
    chain_point *del_point ;
    if(*chain_head == 0)  //链表为空
    {
        return 1;
    }

    while(*chain_head) 
    {
        if((*chain_head)->data == data) //删除的数据区一致
        {
           del_point =  *chain_head;
           if(del_point->prev)  
           {
                *chain_head =  del_point->prev; //回到前一个节点
                (*chain_head)->next = del_point->next;
           }
          
           if(del_point->next)
           {
                *chain_head = del_point->next;//切到下一个节点
                (*chain_head)->prev = del_point->prev;
           }

          if((del_point->next == 0)&&(del_point->prev))  //是最后一个节点
          {
                *chain_head = 0;
          }
          free(del_point);
				  return 0;
        }
        *chain_head = (*chain_head)->next;
    }
    return 0;
}


