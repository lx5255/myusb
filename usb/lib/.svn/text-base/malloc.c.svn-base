#include "my_malloc.h"
#include "config.h"
//typedef unsigned char u8; 
//typedef unsigned short u16;
//typedef unsigned long u32;
typedef struct 
{
	u32 user_flag:3;
	u32 block_size:29;
}block_head;
typedef struct
{
	u32 mem_size;
	u32 free_size;
	u8 *start_mem;
	u8 *end_mem;
}mem_list;
static mem_list mem_info =
{
	0,0,0,0
};

void *my_malloc(u32 size)
{
		block_head *memptr, *last_ptr = 0;
		if(mem_info.start_mem == 0) {
				return 0;
		}
		if(size%4) {
				size += 4 - size%4; //保证4个byte对齐
		}

		memptr = (block_head *)mem_info.start_mem;
		while(1) {
				//		printf("block 0x%x  size:%d\n", memptr, memptr->block_size);
				if(memptr >= (mem_info.end_mem - sizeof(block_head))){ //历遍内存区没有找到合适的内存块
						return 0;
				} 

				if(memptr->user_flag==0){
						if((last_ptr)&&(last_ptr->user_flag==0)){//上一块内存也为空
								last_ptr->block_size += memptr->block_size + sizeof(block_head);//合并内存块
								memptr = last_ptr;
								last_ptr = 0;
								//				printf("e");
								continue;
						} else {
								//				printf("m");
								if(memptr->block_size>=size) {
										void *buff =(u8 *)memptr+sizeof(block_head);
										memptr->user_flag = 1; //置位使用标志
										if((memptr->block_size - size) > 4) {
												block_head *split_block = (u8 *)memptr + sizeof(block_head) + size;  //将大块分割
												split_block->user_flag = 0;
												split_block->block_size = memptr->block_size - sizeof(block_head) - size;
												memptr->block_size = size;
												//						printf("split 0x%x size:%d\n", split_block, split_block->block_size);
										}

										return buff;  
								}
						}
				}
				last_ptr = memptr; 
				memptr = (u8 *)memptr + memptr->block_size + sizeof(block_head);//下一块内存
				//		printf("next ");
		}
		return 0;
}

void my_free(void *mem_buff)
{
	block_head *memptr = 0;

	if(mem_buff) {
		memptr = (u8  *)mem_buff - sizeof(block_head);
		if(memptr->user_flag == 1)
			memptr->user_flag = 0;
	}
	
}

u8  init_mem_alloc(void *buff, u32 size)
{
		block_head *mempr = 0;
		if(buff == 0)
				return 2;
		if(size>(BIT(29)-1))  //内存太大，超过管理范围
				size = BIT(29)-1;

		mem_info.start_mem	= buff;
		mem_info.mem_size		= size;
		mem_info.free_size	= size;
		mem_info.end_mem		= buff+size;
		mempr								= (block_head *)mem_info.start_mem;
		mempr->user_flag		= 0;
		mempr->block_size		= size - sizeof(block_head);
		//	printf("sizeof block head :%d\n", sizeof(block_head));
		return 0;
}
/*
u8 *alloc_buff[512];
int main()
{
	u8 *p;
	init_mem_alloc(alloc_buff, sizeof(alloc_buff));
	do{
		p = my_malloc(0x30);
		p = my_malloc(0x30);
		printf("p :0x%x\n", p);
		my_free(p);
	}while(p);

	return 0;
}
*/ 
