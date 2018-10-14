#include "config.h"

/*----------------------------------------------------------------
**	����	��int str_to_int(const char *str)
**	����	���������ַ���ת����int��
**	����	��str�������ַ���
**	����ֵ��int����
**	��ע	��
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
**	����	��double str_to_double(const char *str)
**	����	���������ַ���ת����double��
**	����	��str�������ַ���
**	����ֵ: double����
**	��ע	��
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
**	����	��char *cpstr_until(char *str_o, const char *str_i, const char cmpchar)
**	����	�������ַ�����֪����⵽ĳ���ַ�Ϊֹ
**	����	��str_o��Ҫ���������ַ���  str_i��Ҫ�������ַ���  cmpchar��Ҫ�����ַ�
**	����ֵ: ��⵽���ַ������ڵ�ַ
**	��ע	����û�м�⵽�ַ���ʱ�������ַ���ĩβ�Զ�����
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
//���ַ���ת����int
int hex_to_int(const char *str)
{
	u32 num = 0;
	
	while((*str != 'x') && (*str != 'X')) //�жϳ���0x��
	{
		str++;
		if(*str == 0)  //�ﵽ�ַ���ĩβ
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

//��ȡָ��λ����ֵ��С
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
//�������תС��
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
//�Ƚ������ַ����Ƿ����
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

