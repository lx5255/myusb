#ifndef _CBUFF_
#define _CBUFF_
typedef struct c_buff_struct 
{
	u8 *buff;	
	u8 *start;
	u8 *end;
	u8 *read_ptr;
	u8 *write_ptr;
	u16 buff_len;
	u16 data_len;
	volatile u8 flag;
}c_buff_struct;

#define FULL_FLAG  (0x01)
#define MUTU_FLAG  (0x02)

c_buff_struct *new_cbuff(u16 size);
void free_cbuff(c_buff_struct **cbuff);
u16 read_cbuff(c_buff_struct *cbuff, void *buf, u16 len);
u16 write_cbuff(c_buff_struct *cbuff, void *buf, u16 len);
u8 check_cbuff_mutu(c_buff_struct *cbuff);
void flush_cbuff(c_buff_struct *cbuff);
u16 get_cbuff_remain(c_buff_struct *c_buff);
u16 get_cbuff_data_size(c_buff_struct *cbuff);
u16 read_cbuff_string(c_buff_struct *cbuff, char *string, u16 len);
#endif
