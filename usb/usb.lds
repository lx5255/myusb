
SECTIONS {
    . = 0x30000000;
		__code_start = .;
    .text ALIGN(4):{start.o  *(.text)}
    .rodata ALIGN(4) : {*(.rodata*)} 

    .dev_list ALIGN(4) : 
		{
				_dev_list_start = .;
				*(.dev_list)
		} 
		_dev_list_end = .;

		.cmd_list ALIGN(4):
		{
				__cmd_list_start = .;
				*(.cmd_list)
		}
		__cmd_list_end = .;


    .data ALIGN(4)   :{ *(.data) }

    .task_list ALIGN(4)   :
		{ 
				__task_list_start = .;
				*(.task_list) 
		}
		__task_list_end = .;

    __bss_start = .;
    .bss ALIGN(4)  : { *(.bss)  *(COMMON) }
    __bss_end = .;

		__stack_start__ = .;
}
