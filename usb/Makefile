
CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

code :=  $(wildcard  ./usb/*.c)
code +=  $(wildcard  ./*.c) 
code +=  $(wildcard  ./uart/*.c) 
code +=  $(wildcard  ./cpu/*.c) 
code +=  $(wildcard  ./cmd/*.c) 
code +=  $(wildcard  ./cbuff/*.c) 
code +=  $(wildcard  ./chain/*.c) 
code +=  $(wildcard  ./comm/*.c) 
code +=  $(wildcard  ./msg/*.c) 
code +=  $(wildcard  ./sddrv/*.c) 
code +=  $(wildcard  ./task_manage/*.c) 
code +=  $(wildcard  ./dev_manage/*.c) 

#code +=  $(wildcard  ./lib/*.c) 

INCLUDEDIR 	:=-I $(shell pwd)
INCLUDEDIR 	+=-I $(shell pwd)/comm
INCLUDEDIR 	+=-I $(shell pwd)/usb/
INCLUDEDIR 	+=-I $(shell pwd)/cpu/
INCLUDEDIR 	+=-I $(shell pwd)/uart/
INCLUDEDIR 	+=-I $(shell pwd)/cmd/
INCLUDEDIR 	+=-I $(shell pwd)/cbuff/
INCLUDEDIR 	+=-I $(shell pwd)/chain/
INCLUDEDIR 	+=-I $(shell pwd)/msg/
INCLUDEDIR 	+=-I $(shell pwd)/sddrv/
INCLUDEDIR  +=-I $(shell pwd)/task_manage/
INCLUDEDIR  +=-I $(shell pwd)/dev_manage/
INCLUDEDIR  +=-I $(shell pwd)/include_lib/
INCLUDEDIR  +=-I /usr/lib/gcc/i486-linux-gnu/4.4/include
INCLUDEDIR  +=-I /usr/include

CFLAGS 			:= -WALL -O2
CPPFLAGS   	:= -nostdinc $(INCLUDEDIR)
CPPLIB 			:= -L/tools/gcc-3.4.5-glibc-2.3.6/arm-linux/lib/-static -lgcc_s  
objs 				:= ${patsubst %.c, %.o,$(code)} ./lib/libc.a start.o lib/malloc.o

LDFLASG     := -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc
#LDFLASG    += -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lm

usb.bin: $(objs)
	${LD} -Tusb.lds  -o usb_elf   $^ ${LDFLASG}
	${OBJCOPY} -O binary -S usb_elf $@
	${OBJDUMP} -D -m arm usb_elf > usb.dis
	arm-linux-nm -v -l usb_elf > usb.map
	dnw usb.bin
%.o:%.c
	${CC}  $(CPPFLAGS) $(CFLAGS)  -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
clean:
	rm -f usb.bin usb_elf usb.dis *.o 
	find . -depth -name '*.o' -type f -print -exec rm -rf {} \; 

back:
	bash back.sh
