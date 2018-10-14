#ifndef _COMM_
#define _COMM_

#define PCLK	50000000L

typedef unsigned char  bool, u8;
typedef unsigned long  u32;
typedef unsigned short u16 ;
typedef volatile unsigned long  reg32;
typedef volatile unsigned short  reg16;
typedef volatile unsigned char  reg8;
typedef char s8;
typedef short s16;
typedef long S32;

#define CLCKCON *(volatile unsigned int *)0x4C00000C
#define BIT(x) (0x01<<x)
#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define NULL (void *)0

void delay(u32 time);
char *cpstr_until(char *str_o, const char *str_i, const char cmpchar);
int str_to_int(const char *str);
double str_to_double(const char *str);
int hex_to_int(const char *str);
u32 get_bit_value(void *in_vlaue, u16 bit, u8 num);
u32 be_to_le_u32(void *prt,u32 word);
u16 be_to_le_u16(void *prt,u16 word);
u8 cmp_str(const char *str_1, const char *str_2);



#endif // _COMM_
