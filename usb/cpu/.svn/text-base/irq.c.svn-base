#include "config.h"
#include "S3C2440.H"
#include "irq.h"

#ifdef BOOT_START
static int_isr_func *isr_handle_array = (int_isr_func)0x33fb06b0;
#else
//static *isr_handle_array = (void *)0x33fb06b0;
#endif

void register_irq(u32 index, u32 sub_index,void (*isr)(void))
{
		INTMSK &= ~BIT(index);						//使能中断
		INTSUBMSK &= ~sub_index;
		isr_handle_array[index] = isr;
}

inline void disable_irq(u32 index)
{
		INTMSK |= BIT(index);						//关闭中断
}

inline void enable_irq(u32 index)
{
		INTMSK &= ~BIT(index);						//使能中断
}

void enable_cpu_int()
{
//		return;
	//	int val;
		__asm__ volatile("mrs r1, cpsr");
		__asm__ volatile("msr cpsr_c, #0xd3");
		__asm__ volatile("bic r1, r1, #0x80");
//		__asm__ volatile("msr cpsr_c, r1");
	/*	__asm
		{
				mrs val, cpsr
				bic val, val, 0x80
				msr cpsr, val
		}*/
		
}

void disable_cpu_int()
{

		__asm__ volatile("mrs r1, cpsr");
//		__asm__ volatile("msr cpsr_c, #0xd3");
	//	__asm__ volatile("mov r1, #192");
		__asm__ volatile("orr r1, r1, #0x80");
//		__asm__ volatile("msr cpsr_c, r1");
}
