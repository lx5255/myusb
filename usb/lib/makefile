
CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

INCLUDEDIR 	:= $(shell pwd)/include
CFLAGS 		:= -Wall -O2
CPPFLAGS   	:= -nostdinc -I$(INCLUDEDIR)
lib := libc.a
objs :=string.o vsprintf.o

libc.a:${objs}
	${AR} -r -o  $@   $^ 
%.o: %.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c -o $@  $<  
	



