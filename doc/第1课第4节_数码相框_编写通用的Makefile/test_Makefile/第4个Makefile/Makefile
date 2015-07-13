objs := a.o b.o

test:$(objs)
	gcc -o test $^

# .a.o.d .b.o.d
dep_files := $(foreach f,$(objs),.$(f).d)			// 将字符串a.o和b.o转换为.a.o.d和.b.o.d，保存在变量dep_files中； 
dep_files := $(wildcard $(dep_files))					// 获取符合格式的文件列表，即.a.o.d  .b.o.d

ifneq ($(dep_files),)													// 如果文件列表不为空，则将依赖文件包含进来
  include $(dep_files)
endif

%.o : %.c 
	gcc -Wp,-MD,.$@.d -c -o $@ $<

clean:
	rm *.o test