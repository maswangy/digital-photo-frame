# := 表示PRJ为立即变量，$(shell basename `pwd`)会被立即展开  
PRJ 			:= $(shell basename `pwd`)
TOPDIR 			:= $(shell pwd)
export TOPDIR

CROSS_COMPILE 	:=
AS				:= $(CROSS_COMPILE)as
LD				:= $(CROSS_COMPILE)ld
CC				:= $(CROSS_COMPILE)gcc
CPP				:= $(CC) -E
AR				:= $(CROSS_COMPILE)ar
NM				:= $(CROSS_COMPILE)nm
STRIP			:= $(CROSS_COMPILE)strip
OBJCOPY			:= $(CROSS_COMPILE)objcopy
OBJDUMP			:= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP

CFLAGS 			:= -Wall -O2
CFLAGS  		+= -I$(PWD)/include 
LDFLAGS 		:= -lfreetype -lm

export CFLAGS LDFLAGS

obj-y += ebook.o
obj-y += bitmap/
obj-y += display/
obj-y += encode/
obj-y += input/

$(PRJ) : 
	make -C ./ -f ./Makefile.build
	$(CC) built-in.o -o $@ $(LDFLAGS)
	
clean:
	rm -f $(shell find -name "*.o")
	rm -f $(PRJ)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(PRJ)
