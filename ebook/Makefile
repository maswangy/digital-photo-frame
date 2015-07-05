# ARM: make CROSS_COMPILE=arm-linux-gnueabihf-
# X86: make
PRJ := ebook 

CROSS_COMPILE :=

CFLAGS 	:= -Wall -O2 -c
CFLAGS  += -I$(PWD)/include

LDFLAGS := -lfreetype -lm

CC 	:= $(CROSS_COMPILE)gcc
LD 	:= $(CROSS_COMPILE)ld

OBJS := ebook.o \
		encode/encode.o \
		encode/ascii.o \
		encode/iso8859.o \
		encode/utf8.o \
		encode/utf16_be.o \
		encode/utf16_le.o \
		bitmap/bitmap.o \
		bitmap/ascii/ascii_8x8.o \
		bitmap/ascii/ascii_8x16.o \
		bitmap/hzk.o \
		bitmap/freetype.o \
		display/display.o \
		display/fb.o \
		input/input.o \
		input/stdin.o \
		input/touchscreen.o \
		
all: $(OBJS)
	$(CC) -o $(PRJ) $^ $(LDFLAGS)

clean:
	rm -f $(PRJ)
	rm -f $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<

