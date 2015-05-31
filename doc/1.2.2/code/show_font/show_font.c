#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/fb.h>
#include <string.h>
#include "font_8x16.h"

unsigned char *fb_mem;
unsigned char *hzk_mem;

int screen_byte_size;
int line_byte_size;
int pixel_byte_size;

void lcd_draw_pixel(int x, int y, int color)
{
    unsigned char *pixel_8 = fb_mem + y * line_byte_size + x * pixel_byte_size;
    unsigned short *pixel_16 = (unsigned short *)(pixel_8);
    unsigned int *pixel_32 = (unsigned int *)(pixel_8);
    int red, green, blue;

    switch(pixel_byte_size) {
    case 1: {
        *pixel_8 = color;
        break;
    }
    case 2: {
        // rgb:5bit 6bit 5bit
        red   = (color >> 16) & 0xff;
        green = (color >> 8) & 0xff;
        blue  = (color >> 0) & 0xff;
        color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
        *pixel_16 = color;
        break;
    }
    case 4: {
        // rgb:8bit 8bit 8bit
        *pixel_32 = color;
        break;
    }
    default:
    {
        printf("fail to suppor %d bpp\n",pixel_byte_size * 8);
    }
    }
}

void lcd_draw_english(int x, int y, unsigned char c)
{
    int i;
    int j;
    unsigned char *dots = (unsigned char *)&fontdata_8x16[c*16];
    unsigned char byte;

    // printf("lcd_draw_english c=%d\n", c);
    for (i = 0; i < 16; i++) {
        byte = dots[i];
        // printf("%x\n", byte);
        for (j = 7; j >= 0; j--) {
            if (byte & (1<<j))
                lcd_draw_pixel(x+7-j, y+i, 0x0000ff);			// blue
            else
                lcd_draw_pixel(x+7-j, y+i, 0x0);
        }
    }
}

void lcd_draw_chinese(int x, int y, unsigned char *s)
{
    int i;
    int j;
    int k;

    int areacode = s[0] - 0xA1;
    int bitcode = s[1] - 0xA1;
    unsigned char *dots = hzk_mem + areacode * 94 * 32 + bitcode * 32;
    unsigned char byte;

    // printf("%x %x\n",s[0], s[1]);
    for (i = 0; i < 16; i++) {          // 16行
        for (k = 0; k < 2; k++) {       // 每行16列
            byte = dots[i*2 + k];
            for(j=7; j>=0; j--)
            {
                if (byte & (1<<j))
                    lcd_draw_pixel(x+8*k+7-j, y+i, 0x0000ff);		// blue
                else
                    lcd_draw_pixel(x+8*k+7-j, y+i, 0x0);
            }
        }
    }

}

int main(int argc, char **argv)
{
    int fd_fb;
    int fd_hzk;
    struct stat hzk_stat;
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;

    int i,j,k;
    unsigned char c;
    unsigned char *s = "你的时间有限，所以不要为别人而活。不要被" \
            "教条所限，不要活在别人的观念里。不要让别人的意见左右自己"  \
            "内心的声音。最重要的是，勇敢的去追随自己的心灵和直觉，只有" \
            "自己的心灵和直觉才知道你自己的真实想法，其他一切都是次要。" \
            "活着就是为了改变世界，难道还有其他原因吗？";

    // open fb
    if ((fd_fb=open("/dev/fb0", O_RDWR)) == -1) {
        printf("fail to open /dev/fb0\n");
        return -1;
    }
    // get fb var&fix info
    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var) == -1) {
        printf("fail to ioctl var\n");
        return -1;
    }
    if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &fix) == -1) {
        printf("fail to ioctl fix\n");
        return -1;
    }

    printf("bpp:%d\n", var.bits_per_pixel);
    printf("xres:%d\n", var.xres);
    printf("yres:%d\n", var.yres);
    screen_byte_size = var.xres * var.yres * var.bits_per_pixel / 8;
    line_byte_size = var.xres * var.bits_per_pixel / 8;
    pixel_byte_size = var.bits_per_pixel / 8;

    // map fb
    if ((fb_mem = mmap(NULL, screen_byte_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_fb, 0)) == MAP_FAILED) {
        printf("fail to mmap fb\n");
        return -1;
    }
    // not clear fb so that we can see debug info on X86 terminal
    // memset(fb_mem, 0, screen_byte_size);

    // open HZK
    if ((fd_hzk = open("HZK16", O_RDONLY)) == -1) {
        printf("fail to open HZK16\n");
        return -1;
    }
    // get HZK stat
    if (fstat(fd_hzk, &hzk_stat) == -1) {
        printf("fail to fstat fd_hzk\n");
        return -1;
    }
    // map HZK
    if ((hzk_mem = mmap(NULL, hzk_stat.st_size, PROT_READ, MAP_SHARED, fd_hzk, 0)) == MAP_FAILED) {
        printf("fail to mmap HZK16\n");
        return -1;
    }

    // draw chinese:16bit x 16byte
    j = var.xres/2;
    k = var.yres/2;
    for (i=0; s[i]!='\0'; i+=2) {
        lcd_draw_chinese(j, k, &(s[i]));
        j = j + 16;
        if (j > (var.xres-16)) {
            k += 16;
            j = var.xres/2;
        }
    }

    // draw char:8bit x 16byte
    j = var.xres/2;
    k = k + 16;
    for (c = 'A'; c <= 'z'; c++) {
        lcd_draw_english(j, k, c);
        j = j + 8;
        if (j > (var.xres-16)) {
            k += 16;
            j = var.xres/2;
        }
    }
}


