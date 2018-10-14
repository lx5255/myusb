#ifndef _CHAIN_
#define _CHAIN_

typedef struct
{
    void *next, *prev;
    void *data;
}chain_point;

u8 add_chain_point(chain_point **chain_head, void *data);
u8 del_chain_point(chain_point **chain_head, void *data);

#endif
