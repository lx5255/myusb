#ifndef _INTRUPT_
#define _INTRUPT_
// Interrupt Pending Bit
#define INT_EINT0            ( 0)
#define INT_EINT1            ( 1)
#define INT_EINT2            ( 2)
#define INT_EINT3            ( 3)
#define INT_EINT4_7          ( 4)
#define INT_EINT8_23         ( 5)
#define INT_CAM              ( 6)
#define INT_nBAT_FLT         ( 7)
#define INT_TICK             ( 8)
#define INT_WDT_AC97         ( 9)
#define INT_TIMER0           (10)
#define INT_TIMER1           (11)
#define INT_TIMER2           (12)
#define INT_TIMER3           (13)
#define INT_TIMER4           (14)
#define INT_UART2            (15)
#define INT_LCD              (16)
#define INT_DMA0             (17)
#define INT_DMA1             (18)
#define INT_DMA2             (19)
#define INT_DMA3             (20)
#define INT_SDI              (21)
#define INT_SPI0             (22)
#define INT_UART1            (23)
#define INT_NFCON            (24)
#define INT_USBD             (25)
#define INT_USBH             (26)
#define INT_IIC              (27)
#define INT_UART0            (28)
#define INT_SPI1             (29)
#define INT_RTC              (30)
#define INT_ADC              (31)

#define INT_SUB_RXD0         (1 <<  0)
#define INT_SUB_TXD0         (1 <<  1)
#define INT_SUB_ERR0         (1 <<  2)
#define INT_SUB_RXD1         (1 <<  3)
#define INT_SUB_TXD1         (1 <<  4)
#define INT_SUB_ERR1         (1 <<  5)
#define INT_SUB_RXD2         (1 <<  6)
#define INT_SUB_TXD2         (1 <<  7)
#define INT_SUB_ERR2         (1 <<  8)
#define INT_SUB_TC           (1 <<  9)
#define INT_SUB_ADC_S        (1 << 10)
#define INT_SUB_CAM_C        (1 << 11)
#define INT_SUB_CAM_P        (1 << 12)
#define INT_SUB_WDT          (1 << 13)
#define INT_SUB_AC97         (1 << 14)
#define INT_SUB_ALLMSK       (0x000007FF)

#define ClearPending(bit)    { SRCPND = BIT(bit);    \
                               INTPND = INTPND; }
#define ClearSubPending(bit)  {SUBSRCPND = bit;}
#define DIS_CPU_INT			//	disable_cpu_int();
#define EN_CPU_INT			//  enable_cpu_int();
typedef void (*int_isr_func)(void);


void register_irq(u32 index, u32 sub_index,void (*isr)(void));
inline void disable_irq(u32 index);
inline void enable_irq(u32 index);
void disable_cpu_int();
void enable_cpu_int();
#endif
