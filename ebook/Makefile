# := 表示PRJ为立即变量，$(shell basename `pwd`)会被立即展开  
PRJ 			:= $(shell basename `pwd`)

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

CFLAGS 			:= -Wall -O2 -c
CFLAGS  		+= -I$(PWD)/include 
LDFLAGS 		:= -lfreetype -lm

export CFLAGS LDFLAGS

$(PRJ):built-in.o 
	# 编译出项目可执行文件
	$(CC) -o $@ $^ $(LDFLAGS)
	
built-in.o：
	# 编译出顶层目录的built-in.o
	make -C ./ -f ./Makefile.build

clean:
	rm -f $(PRJ)