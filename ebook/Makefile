PRJ := ebook 

CROSSCOMPILE :=

CFLAGS 	:= -Wall -O2 -c
CFLAGS  += -I$(PWD)/include

LDFLAGS := -lm -lfreetype

CC 	:= $(CROSSCOMPILE)gcc
LD 	:= $(CROSSCOMPILE)ld

OBJS := ebook.o

all: $(OBJS)
	$(CC) $(LDFLAGS) -o $(PRJ) $^

clean:
	rm -f $(PRJ)
	rm -f $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<

