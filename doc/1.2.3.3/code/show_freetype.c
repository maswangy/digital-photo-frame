#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/fb.h>
#include <string.h>
#include <math.h>
#include <wchar.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

unsigned char *fb_mem;
unsigned char *hzk_mem;

int screen_byte_size;
int line_byte_size;
int pixel_byte_size;
int fd_fb;
struct fb_var_screeninfo var;
struct fb_fix_screeninfo fix;

void lcd_draw_pixel(int x, int y, int color)
{
    unsigned char *pixel_8 = fb_mem + y * line_byte_size + x * pixel_byte_size;
    unsigned short *pixel_16 = (unsigned short *)(pixel_8);
    unsigned int *pixel_32 = (unsigned int *)(pixel_8);
    int red,green,blue;

    switch(pixel_byte_size) {
    case 1:
        *pixel_8 = color;
        break;
    case 2:
        // rgb:5bit 6bit 5bit
        red   = (color >> 16) & 0xff;
        green = (color >> 8) & 0xff;
        blue  = (color >> 0) & 0xff;
        color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
        *pixel_16 = color;
        break;
    case 4:
        // rgb:8bit 8bit 8bit
        *pixel_32 = color;
        break;
    default:
        printf("Fail to support %bpp\n",pixel_byte_size * 8);
    }
}

void draw_bitmap(FT_Bitmap*  bitmap, FT_Int x, FT_Int y)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;


    for (i = x, p = 0; i < x_max; i++, p++) {
        for (j = y, q = 0; j < y_max; j++, q++) {
            if (i < 0      || j < 0       ||
                    i >= var.xres || j >= var.yres)
                continue;
            lcd_draw_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
        }
    }
}


int main(int argc, char **argv)
{
    double angle;
    int n;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <font> <angle>\n", argv[0]);
        exit(1);
    }

    // open fb
    if ((fd_fb=open("/dev/fb0", O_RDWR)) == -1) {
        printf("Fail to open /dev/fb0\n");
        return -1;
    }

    // get fb var&fix info
    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var) == -1) {
        printf("Fail to ioctl var\n");
        return -1;
    }
    if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &fix) == -1) {
        printf("Fail to ioctl fix\n");
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
        printf("Fail to mmap fb\n");
        return -1;
    }

    // clear fb
    memset(fb_mem, 0, screen_byte_size);

    FT_Library    library;
    FT_Face       face;
    FT_GlyphSlot  slot;
    FT_Matrix     matrix;                 /* transformation matrix */
    FT_Vector     pen;                    /* untransformed origin  */
    FT_Error      error;

    // �ÿ��ַ�һ���ַ���4���ֽڱ���
    wchar_t *chinese_str = L"��ΰ��ABC";
    // ��ת�Ƕ�
    angle = (1.0 * strtoul(argv[2],NULL,0)/ 360) * 3.14159 * 2;

    FT_Init_FreeType(&library);              /* initialize library */
    FT_New_Face(library, argv[1], 0, &face); /* create face object */
    FT_Set_Pixel_Sizes(face, 24, 0);

    // glyph��ۣ�FT_GlyphSlot
    slot = face->glyph;
    /* set up matrix */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

    /* the pen position in 26.6 cartesian space coordinates; */
    /* start at (0,40) relative to the upper left corner  */
    pen.x = 0 * 64;
    pen.y = (var.yres- 40) * 64;

    for (n = 0; n < wcslen(chinese_str); n++) {
        // ���ñ任��������ת���ƶ�
        FT_Set_Transform(face, &matrix, &pen);

        // ��glyph���ص�glyph����У�����glyphת��Ϊbitmap(λͼ)��������slot->bitmap�С�
        error = FT_Load_Char(face, chinese_str[n], FT_LOAD_RENDER );
        if (error)
            continue;                 /* ignore errors */
        /* now, draw to our target surface (convert position) */
        draw_bitmap(&slot->bitmap, slot->bitmap_left, var.yres - slot->bitmap_top);

        /* increment pen position */
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}


