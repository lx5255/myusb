
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
@ �ж��������������У���Reset��HandleIRQ�⣬�����쳣��û��ʹ��
@******************************************************************************       
    @ldr sp, =4096           @ ����ջָ�룬���¶���C����������ǰ��Ҫ���ջ
    @bl  disable_watch_dog   @ �ر�WATCHDOG������CPU�᲻������
    @bl  clock_init          @ ����MPLL���ı�FCLK��HCLK��PCLK
   @ bl  memsetup            @ ���ô洢��������ʹ��SDRAM
   @ bl  nand_init           @ ��ʼ��NAND Flash
@	bl  clean_bss           @ ���bss�Σ�δ��ʼ�����ֵΪ0��ȫ��/��̬����������bss��
@	msr cpsr_c, #0xd2       @ �����ж�ģʽ
 @   ldr sp, =0x30400000     @ �����ж�ģʽջָ��

@    msr cpsr_c, #0xdf       @ ����ϵͳģʽ
   ldr sp, =0x30200000     @ ����ϵͳģʽջָ�룬
@	 ldr pc, = sys_init
   bl sys_init
   ldr pc, = main       @ ����main����
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