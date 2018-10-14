
.extern    main
.extern		sys_init
.text 

.global start 



.equ UND_Stack_Size,       0x00000000
.equ SVC_Stack_Size,       0x00000008
.equ ABT_Stack_Size,       0x00000000
.equ FIQ_Stack_Size,       0x00000000
.equ IRQ_Stack_Size,       0x00000400
.equ USR_Stack_Size,       0x00040000

.equ ISR_Stack_Size,      (UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + \
                         FIQ_Stack_Size + IRQ_Stack_Size)


#define  Stack_Mem        USR_Stack_Size
#define  __initial_sp	 ISR_Stack_Size

;// <h> Heap Configuration
;//   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF>
;// </h>
.equ Heap_Size,            0x00040000

#define Heap_Mem          Heap_Size
start:
@******************************************************************************       
@ 中断向量，本程序中，除Reset和HandleIRQ外，其它异常都没有使用
@******************************************************************************       
    @ldr sp, =4096           @ 设置栈指针，以下都是C函数，调用前需要设好栈
    @bl  disable_watch_dog   @ 关闭WATCHDOG，否则CPU会不断重启
    @bl  clock_init          @ 设置MPLL，改变FCLK、HCLK、PCLK
   @ bl  memsetup            @ 设置存储控制器以使用SDRAM
   @ bl  nand_init           @ 初始化NAND Flash
@	bl  clean_bss           @ 清除bss段，未初始化或初值为0的全局/静态变量保存在bss段
@	msr cpsr_c, #0xd2       @ 进入中断模式
 @   ldr sp, =0x30400000     @ 设置中断模式栈指针

@    msr cpsr_c, #0xdf       @ 进入系统模式
   ldr sp, =0x30200000     @ 设置系统模式栈指针，
@	 ldr pc, = sys_init
   bl sys_init
   ldr pc, = main       @ 调用main函数
halt_loop:
    b   halt_loop

clean_bss:
	ldr r0, =__bss_start
	ldr r1, =__bss_end
bss_loop:
	mov r3, #0
	str r3, [r0], #4
	cmp r0, r1
	bcc bss_loop
	mov pc, lr 
