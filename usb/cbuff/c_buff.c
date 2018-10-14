#include "config.h"
#include "c_buff.h"

/*
c_buff_struct  *c_buff = NULL;
const u8 str1[] = "hello word 123456789";


int main(void)
{
	u16 res;
	u8 read_buff[50];

	c_buff =new_cbuff(100);
	if(c_buff == NULL)
	{
		return 0;
	}
	res =  write_cbuff(c_buff, (u8 *)str1 , sizeof(str1));
	res =  write_cbuff(c_buff, (u8 *)str1 , sizeof(str1));
	res =  write_cbuff(c_buff, (u8 *)str1 , sizeof(str1));
	res =  write_cbuff(c_buff, (u8 *)str1 , sizeof(str1));
	res =  write_cbuff(c_buff, (u8 *)str1 , sizeof(str1));
	res =  write_cbuff(c_buff, (u8 *)str1 , sizeof(str1));
	printf("res:%d data len:%d\n", res, c_buff->data_len);

	res =  read_cbuff(c_buff, (u8 *)read_buff ,105);
	printf("res:%d data len:%d  %s\n", res, c_buff->data_len, read_buff);

	memset(read_buff, 0x00, sizeof(str1));
res =  read_cbuff(c_buff, (u8 *)read_buff , sizeof(str1));
	printf("res:%d data len:%d  %s\n", res, c_buff->data_len, read_buff);
	
	memset(read_buff, 0x00, sizeof(str1));
	res =  read_cbuff(c_buff, (u8 *)read_buff , sizeof(str1));
	printf("res:%d data len:%d  %s\n", res, c_buff->data_len, read_buff);
	memset(read_buff, 0x00, sizeof(str1));
res =  read_cbuff(c_buff, (u8 *)read_buff , sizeof(str1));
	printf("res:%d data len:%d  %s\n", res, c_buff->data_len, read_buff);
	memset(read_buff, 0x00, sizeof(str1));
res =  read_cbuff(c_buff, (u8 *)read_buff , sizeof(str1));
	printf("res:%d data len:%d  %s\n", res, c_buff->data_len, read_buff);
	memset(read_buff, 0x00, sizeof(str1));
		res =  read_cbuff(c_buff, (u8 *)read_buff , sizeof(str1));
	printf("res:%d data len:%d  %s\n", res, c_buff->data_len, read_buff);
	return 0;
}

*/

//创建cbuff句柄
//返回创建的句柄
//size cbuff的大小
c_buff_struct *new_cbuff(u16 size) {
		c_buff_struct *new_buf = NULL;
		new_buf = (c_buff_struct *)malloc(sizeof(c_buff_struct));
		if(new_buf == NULL) {
				return new_buf;
		}

		new_buf->buff = malloc(size);
		if(new_buf->buff == NULL) {
				free(new_buf);
				return 0;
		}

		new_buf->buff_len = size;
		new_buf->data_len = 0;
		new_buf->flag = 0;
		new_buf->start = new_buf->buff;
		new_buf->end = new_buf->buff + new_buf->buff_len ;
		new_buf->read_ptr = new_buf->buff;
		new_buf->write_ptr = new_buf->buff;
		return new_buf;
}
//释放cbuff句柄
//cbuff 句柄指针地址
void free_cbuff(c_buff_struct **cbuff)
{
		if((*cbuff)->buff) {
				free((*cbuff)->buff);
		}

		if(*cbuff) {
				free(*cbuff);
		}

		*cbuff = NULL;
}
//从cbuff中读取指定大小的数据
//返回实际读取的长度
u16 read_cbuff(c_buff_struct *cbuff, void *buf, u16 len)
{
		u16 size = cbuff->data_len > len ? len: cbuff->data_len; //获取读取大小
		u8 *buff = buf;
		if(size == 0) {
				return 0;
		}
		if(cbuff->flag & MUTU_FLAG){ //内存正在使用
				//		return 0;
		}
		DIS_CPU_INT
		cbuff->flag |= MUTU_FLAG;

		if(cbuff->write_ptr > cbuff->read_ptr) {
				memcpy(buff, cbuff->read_ptr, size);
				cbuff->read_ptr +=  size;
		}else{
				u16 copy_size;
				copy_size = cbuff->end - cbuff->read_ptr;

				if(copy_size >= size){  //没有超越buff边界
						memcpy(buff, cbuff->read_ptr, size);
						cbuff->read_ptr += size;
				}else{  //超越了buff的最大地址
						if(copy_size)
								memcpy(buff, cbuff->read_ptr, copy_size);
						buff +=  copy_size;
						copy_size = size - copy_size; //剩余大小
						memcpy(buff, cbuff->start, copy_size);

						cbuff->read_ptr = cbuff->start + copy_size;
				}
		}

		cbuff->data_len -= size;
		cbuff->flag &= ~(FULL_FLAG|MUTU_FLAG);	
		EN_CPU_INT
		return size;
}
//从cbuff中读取一条字符串
//返回读取到的字符串长度
//len:缓存的最大长度
u16 read_cbuff_string(c_buff_struct *cbuff, char *string, u16 len)
{
	u16 size = cbuff->data_len > len ? len: cbuff->data_len; //获取读取大小
	u8 *buff = string;
	if(size == 0) {
		return 0;
	}
	if(cbuff->flag & MUTU_FLAG){ //内存正在使用
//		return 0;
	}
	DIS_CPU_INT
	cbuff->flag |= MUTU_FLAG;
	for(len = 0;size--;) {
		if(cbuff->read_ptr == cbuff->end) {
				cbuff->read_ptr = cbuff->start;	
		}
		len++;
		*buff = *(cbuff->read_ptr++);
		cbuff->data_len--;
		cbuff->flag &=~FULL_FLAG;
		if(*buff++ == '\0') {
				break;	
		}
	}
	cbuff->flag &=~MUTU_FLAG;
	EN_CPU_INT
 //enable_cpu_int();
//	uprintf("cbuff len %d\n\r", len);
		return len;
}
//向cbuff中写入数据
//返回写入的实际大小
u16 write_cbuff(c_buff_struct *cbuff, void *buf, u16 len)
{
	u16 size = cbuff->buff_len - cbuff->data_len;  //获取剩余空间大小
  u8 *buff = buf;

	size = size>len ? len : size;

	if(size == 0) {
		return 0;
	}

  if(cbuff->flag & MUTU_FLAG){ //内存正在使用
//		return 0;
	}

	DIS_CPU_INT
	cbuff->flag |= MUTU_FLAG;

	if(cbuff->write_ptr < cbuff->read_ptr) {
		memcpy(cbuff->write_ptr, buff, size);
		cbuff->write_ptr += size;
	} else {
		u16 copy_size;
		copy_size = cbuff->end - cbuff->write_ptr;

		if(copy_size >= size){  //没有超越buff边界
			memcpy(cbuff->write_ptr, buff, size);
			cbuff->write_ptr += size;
		} else{//超越了buff的最大地址
		  if(copy_size)
				memcpy(cbuff->write_ptr, buff, copy_size);

			buff += copy_size;
			copy_size = size - copy_size; //剩余大小
			memcpy(cbuff->start, buff, copy_size);

			cbuff->write_ptr = cbuff->start + copy_size;
		}
	}
	cbuff->data_len += size;

	if(cbuff->data_len == cbuff->buff_len) {
		if(cbuff->write_ptr != cbuff->read_ptr){
				uprintf("cbuff len err");
		}
		cbuff->flag |= FULL_FLAG;
	}
	cbuff->flag &=~MUTU_FLAG;
	EN_CPU_INT

	return size;
}
//校验cbuff是否被占用
u8 check_cbuff_mutu(c_buff_struct *cbuff)
{
		return cbuff->flag&MUTU_FLAG;
}
//刷新cbuff里的数据
void flush_cbuff(c_buff_struct *cbuff)
{
	if(cbuff->flag&MUTU_FLAG) {
		return;
	}
	cbuff->data_len = 0;
	cbuff->flag &= ~FULL_FLAG;
	cbuff->read_ptr = cbuff->start;
	cbuff->write_ptr = cbuff->start;
}

inline u16 get_cbuff_data_size(c_buff_struct *cbuff)
{
		return cbuff->data_len; 
}

//获取剩余大小
inline u16 get_cbuff_remain(c_buff_struct *cbuff)
{
		return cbuff->buff_len - cbuff->data_len;  //获取剩余空间大小
}
