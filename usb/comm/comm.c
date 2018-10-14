#include "config.h"

/*----------------------------------------------------------------
**	函数	：int str_to_int(const char *str)
**	功能	：将数字字符串转换成int型
**	参数	：str：数字字符串
**	返回值：int数字
**	备注	：
-------------------------------------------------------------------*/
int str_to_int(const char *str)
{
	u32 num;
	num = *str - '0';
	while(*++str)
	{
		if((*str<'0')||(*str>'9'))
		{
			break;
		}			
		num = num*10 + *str - '0';
	}
	return num;
}
/*----------------------------------------------------------------
**	函数	：double str_to_double(const char *str)
**	功能	：将数字字符串转换成double型
**	参数	：str：数字字符串
**	返回值: double数字
**	备注	：
-------------------------------------------------------------------*/
double str_to_double(const char *str)
{
	double data = 0.0, flaot_flag = 0.1;
	u8 int_flag = 0;
	while(*str)
	{	
		if(*str == '.')
		{
			int_flag = 1;
		}
		else if((*str<= '9')&&(*str >= '0'))
		{
			if(int_flag == 0)
			{
				data = data*10.0 +  (double)(*str - '0');
			}
			else
			{
				data = data +  (double)(*str - '0')*flaot_flag;
				flaot_flag = flaot_flag*0.1;
			}
		}
		else 
		{
			break;
		}
		str++;		
	}
	return data;
}
/*----------------------------------------------------------------
**	函数	：char *cpstr_until(char *str_o, const char *str_i, const char cmpchar)
**	功能	：复制字符串，知道检测到某个字符为止
**	参数	：str_o：要拷贝到的字符串  str_i：要拷贝的字符串  cmpchar：要检测的字符
**	返回值: 检测到的字符串所在地址
**	备注	：当没有检测到字符串时，到达字符串末尾自动结束
-------------------------------------------------------------------*/
char *cpstr_until(char *str_o, const char *str_i, const char cmpchar)
{
	while((*str_i != cmpchar)&&(*str_i))
	{
		*str_o++ = *str_i++;
	}
	*str_o = 0;
	return  (char *)str_i;
}
void delay(u32 time)
{
	u16 i;
	while(time--)
	{
		for(i=0; i<3000; i++)
		;
	}
}
//将字符串转换成int
int hex_to_int(const char *str)
{
	u32 num = 0;
	
	while((*str != 'x') && (*str != 'X')) //判断出“0x”
	{
		str++;
		if(*str == 0)  //达到字符串末尾
		{
			return 0;
		}
	}
	str++;
	while(*str != 0)
	{
		num *= 16;
		if((*str >= '0')&&(*str <= '9'))
		{
			num += *str - '0';
		}
		else if((*str >= 'a')&&(*str <= 'f'))
		{
			num += *str - 'a' + 10;
		}
		else if((*str >= 'A')&&(*str <= 'F'))
		{
			num += *str - 'A' + 10;
		}
		str++;		
	}
	return num;
}

//获取指定位的数值大小
u32 get_bit_value(void *in_vlaue, u16 bit, u8 num)
{
		u8 *p = in_vlaue;
		u32 temp;
		u8 cnt, len;
		u32 return_value = 0;


		if(num > 32 )
		{
				return 0;
		}
		while(bit>7)
		{
				bit -= 8;
				p++;
		}

		cnt = 0;
		while(num)
		{

				if(num>(8-bit))
				{
						len = 8-bit;
				}
				else
				{
						len = num;
				}


				temp =  (*p++)>> bit;
				//  uprintf("0x%x  ",temp);
				if(len)
				{
						temp &= (0x01<<len)-1;
				}
				temp <<= cnt;
				// return_value <<= cnt;         
				return_value |= temp;
				bit -= bit;
				num -= len;
				cnt += len;
				// uprintf("r 0x%x  ",return_value);

		}
		return return_value;    
}
//大端数据转小端
u32 be_to_le_u32(void *prt, u32 word)
{
	u32 cache;
	u8 *p1 = (u8 *)&word;
	u8 *p2 = (u8 *)&cache;

	if(prt)
	{
        p1 = prt;
	}

	p2[0] = p1[3];
	p2[1] = p1[2];
	p2[2] = p1[1];
	p2[3] = p1[0];
	return cache;
}

u16 be_to_le_u16(void *prt, u16 word)
{
	u16 cache;
	u8 *p1 = (u8 *)&word;
	u8 *p2 = (u8 *)&cache;

    if(prt)
	{
        p1 = prt;
	}
	
	p2[0] = p1[1];
	p2[1] = p1[0];
	
	return cache;
}
//比较两个字符串是否相等
u8 cmp_str(const char *str_1, const char *str_2)
{
    while(1)
    {
        if(*str_1 != *str_2)
        {
            return 0;
        }
        if(*str_1 == 0)
        {
            return 1;
        }
        str_1++;
        str_2++;     
    }
}

