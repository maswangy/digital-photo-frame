#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>
#include <stdlib.h>

#define FB_DEVICE_NAME "/dev/fb0"

static int g_fd;
static struct fb_var_screeninfo g_fb_var;
static struct fb_fix_screeninfo g_fb_fix;
static unsigned char *g_fbmem;
static unsigned int g_screen_size;
static unsigned int g_line_width;
static unsigned int g_pixel_width;

static int fb_dev_init(void)
{
	int ret;

	g_fd = open(FB_DEVICE_NAME, O_RDWR);
	if (0 > g_fd) {
		printf("can't open %s\n", FB_DEVICE_NAME);
	}

	ret = ioctl(g_fd, FBIOGET_VSCREENINFO, &g_fb_var);
	if (ret < 0) {
		printf("can't get fb's var\n");
		return -1;
	}

	ret = ioctl(g_fd, FBIOGET_FSCREENINFO, &g_fb_fix);
	if (ret < 0) {
		printf("can't get fb's fix\n");
		return -1;
	}

	g_screen_size = g_fb_var.xres * g_fb_var.yres * g_fb_var.bits_per_pixel / 8;
	g_fbmem = (unsigned char *)mmap(NULL , g_screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
	if (0 > g_fbmem) {
		printf("can't mmap\n");
		return -1;
	}

	g_line_width  = g_fb_var.xres * g_fb_var.bits_per_pixel / 8;
	g_pixel_width = g_fb_var.bits_per_pixel / 8;

	return 0;
}


static int fb_show_pixel(int iX, int iY, unsigned int dwColor)
{
	unsigned char *pucFB;
	unsigned short *pwFB16bpp;
	unsigned int *pdwFB32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;

	if ((iX >= g_fb_var.xres) || (iY >= g_fb_var.yres)) {
		printf("out of region\n");
		return -1;
	}

	pucFB      = g_fbmem + g_line_width * iY + g_pixel_width * iX;
	pwFB16bpp  = (unsigned short *)pucFB;
	pdwFB32bpp = (unsigned int *)pucFB;

	switch (g_fb_var.bits_per_pixel) {
		case 8: {
			*pucFB = (unsigned char)dwColor;
			break;
		}
		case 16: {
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwFB16bpp	= wColor16bpp;
			break;
		}
		case 32: {
			*pdwFB32bpp = dwColor;
			break;
		}
		default: {
			printf("can't support %d bpp\n", g_fb_var.bits_per_pixel);
			return -1;
		}
	}

	return 0;
}

static int fb_clean_screen(unsigned int dwBackColor)
{
	unsigned char *pucFB;
	unsigned short *pwFB16bpp;
	unsigned int *pdwFB32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int i = 0;

	pucFB      = g_fbmem;
	pwFB16bpp  = (unsigned short *)pucFB;
	pdwFB32bpp = (unsigned int *)pucFB;

	switch (g_fb_var.bits_per_pixel) {
		case 8: {
			memset(g_fbmem, dwBackColor, g_screen_size);
			break;
		}
		case 16: {
			iRed   = (dwBackColor >> (16+3)) & 0x1f;
			iGreen = (dwBackColor >> (8+2)) & 0x3f;
			iBlue  = (dwBackColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			while (i < g_screen_size) {
				*pwFB16bpp	= wColor16bpp;
				pwFB16bpp++;
				i += 2;
			}
			break;
		}
		case 32: {
			while (i < g_screen_size) {
				*pdwFB32bpp	= dwBackColor;
				pdwFB32bpp++;
				i += 4;
			}
			break;
		}
		default : {
			printf("can't support %d bpp\n", g_fb_var.bits_per_pixel);
			return -1;
		}
	}

	return 0;
}

static int fb_show_line(int iXStart, int iXEnd, int iY, unsigned char *pucRGBArray)
{
	int i = iXStart * 3;
	int iX;
	unsigned int dwColor;

	if (iY >= g_fb_var.yres)
		return -1;

	if (iXStart >= g_fb_var.xres)
		return -1;

	if (iXEnd >= g_fb_var.xres) {
		iXEnd = g_fb_var.xres;
	}

	for (iX = iXStart; iX < iXEnd; iX++) {
		/* 0xRRGGBB */
		dwColor = (pucRGBArray[i]<<16) + (pucRGBArray[i+1]<<8) + (pucRGBArray[i+2]<<0);
		i += 3;
		fb_show_pixel(iX, iY, dwColor);
	}
	return 0;
}

/* Uage: jpg2rgb <jpg_file>
 */

int main(int argc, char **argv)
{
    // 定义一个decompress结构体
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * infile;
	int row_stride;
	unsigned char *buffer;

	if (argc != 2) {
		printf("Usage: \n");
		printf("%s <jpg_file>\n", argv[0]);
		return -1;
	}

	if (fb_dev_init()) {
		return -1;
	}

	fb_clean_screen(0);

	// 初始化decompress结构体
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// 指定jpg文件
	if ((infile = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", argv[1]);
		return -1;
	}
	jpeg_stdio_src(&cinfo, infile);

	// 读jpg文件头部信息
	jpeg_read_header(&cinfo, TRUE);
	printf("image_width = %d\n", cinfo.image_width);             // 原分辨率
	printf("image_height = %d\n", cinfo.image_height);
	printf("num_components = %d\n", cinfo.num_components);

	// 设置缩放比例为M/N
	printf("enter scale M/N:\n");
	scanf("%d/%d", &cinfo.scale_num, &cinfo.scale_denom);
	printf("scale to : %d/%d\n", cinfo.scale_num, cinfo.scale_denom);

	// 开始解压
	jpeg_start_decompress(&cinfo);
	printf("output_width = %d\n", cinfo.output_width);           // 解压后后的分辨率
	printf("output_height = %d\n", cinfo.output_height);
	printf("output_components = %d\n", cinfo.output_components); // 每个像素包含几个component，每个component占用1byte

	// 一行的长度
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = malloc(row_stride);

	// 逐行扫描
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, &buffer, 1);
		fb_show_line(0, cinfo.output_width, cinfo.output_scanline, buffer);
	}

	free(buffer);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 0;
}

