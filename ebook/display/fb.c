/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : fb.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <string.h>
#include "config.h"
#include "display.h"

static unsigned char *fb_mem;
static struct fb_var_screeninfo var;
static struct fb_fix_screeninfo fix;

static int fb_init(void)
{
    int fd_fb;
    int screen_bytes;

    // open fb
    if ((fd_fb = open("/dev/fb0", O_RDWR)) == -1) {
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

    screen_bytes = var.xres * var.yres * var.bits_per_pixel / 8;
    if ((fb_mem = mmap(NULL, screen_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, fd_fb, 0)) == MAP_FAILED) {
        printf("fail to mmap fb\n");
        return -1;
    }
    return 0;
}
static int fb_draw_pixel(int x, int y, unsigned int color)
{
    int pixel_bytes = var.bits_per_pixel / 8;
    int line_bytes = var.xres * var.bits_per_pixel / 8;
    unsigned char *pixel_8 = fb_mem + y * line_bytes + x * pixel_bytes;
    unsigned short *pixel_16 = (unsigned short *)(pixel_8);
    unsigned int *pixel_32 = (unsigned int *)(pixel_8);
    int red, green, blue;

    switch(pixel_bytes) {
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
    default: {
        PRINT_ERR("Unsupported bpp:%d\n", var.bits_per_pixel);
    }
    }
    return 0;
}

int fb_clear_screen(int color)
{
    int red, green, blue;
    int screen_bytes = var.xres * var.yres * var.bits_per_pixel / 8;
    unsigned char *pixel_8 = fb_mem;
    unsigned short *pixel_16 = (unsigned short *)(pixel_8);
    unsigned int *pixel_32 = (unsigned int *)(pixel_8);
    int i = 0;

    switch(var.bits_per_pixel) {
    case 8: {
        memset(fb_mem, color, screen_bytes);
        break;
    }
    case 16: {
        // rgb:5bit 6bit 5bit
        red   = (color >> 16) & 0xff;
        green = (color >> 8) & 0xff;
        blue  = (color >> 0) & 0xff;
        color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
        while (i < screen_bytes) {
            *pixel_16++ = color;
            i += 2;
        }
        break;
    }
    case 32: {
        while (i < screen_bytes) {
            *pixel_32++ = color;
            i += 4;
        }
        break;
    }
    default: {
        break;
        PRINT_ERR("Unsupported bpp:%d\n", var.bits_per_pixel);
        return -1;
    }
    }
    return 0;
}
static struct display_ops fb_display_ops = {
        .name = "fb",
        .type = DISPLAY_FB,
        .draw_pixel = fb_draw_pixel,
        .clear_screen = fb_clear_screen,
};

int fb_display_init(void)
{
    fb_init();
    fb_display_ops.xres = var.xres;
    fb_display_ops.yres = var.xres;
    if (register_display_ops(&fb_display_ops) == -1) {
        PRINT_ERR("fail to register %s display ops\n", fb_display_ops.name);
        return -1;
    }
    return 0;
}

int fb_display_exit(void)
{
    if (deregister_display_ops(&fb_display_ops) == -1) {
        PRINT_ERR("fail to deregister %s display ops\n", fb_display_ops.name);
        return -1;
    }
    return 0;
}
