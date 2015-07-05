// gcc -finput-charset=GBK -fexec-charset=UTF-8 show_lines_left.c -o show_lines_left -lfreetype -lm
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
int fd_fb;
struct fb_var_screeninfo var;
struct fb_fix_screeninfo fix;

int screen_byte_size;
int line_byte_size;
int pixel_byte_size;

void lcd_draw_pixel(int x, int y, int color)
{
    unsigned char *pixel_8 = fb_mem + y * line_byte_size + x * pixel_byte_size;
    unsigned short *pixel_16 = (unsigned short *)(pixel_8);
    unsigned int *pixel_32 = (unsigned int *)(pixel_8);
    int red,green,blue;

    switch (pixel_byte_size)
    {
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
        printf("Fail to support %dbpp\n",pixel_byte_size * 8);
    }
}

void draw_bitmap(FT_Bitmap*  bitmap, FT_Int x, FT_Int y)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;

    for (i = x, p = 0; i < x_max; i++, p++ ) {
        for (j = y, q = 0; j < y_max; j++, q++) {
            if (i < var.xres/2 || j < 0 || i >= var.xres || j >= var.yres)
                continue;
            lcd_draw_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
        }
    }
}

int main(int argc, char **argv)
{
    double angle;
    int n = 0;
    int i = 0;
    int max = 0;
    int min = 1000;

    if (argc != 3) {
        fprintf ( stderr, "usage: %s <font> <angle>\n", argv[0] );
        exit( 1 );
    }

    // open fb
    if ((fd_fb = open("/dev/fb0", O_RDWR)) == -1) {
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
    FT_BBox bbox;
    FT_Glyph  glyph;

    // 用宽字符，一个字符用4个字节保存
    wchar_t *chinese_str[] = {L"吴伟东gif",L"本科123",L"嵌入式软件工程师",L"华南师范大学"};
    // 旋转角度
    angle = (1.0 * strtoul(argv[2],NULL,0) / 360) * 3.14159 * 2;

    FT_Init_FreeType(&library);              /* initialize library */
    FT_New_Face(library, argv[1], 0, &face); /* create face object */
    FT_Set_Pixel_Sizes(face, 24, 0);

    // glyph插槽：FT_GlyphSlot
    slot = face->glyph;

    /* set up matrix */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

    // (0, 24)
    pen.x = var.xres / 2 * 64;
    pen.y = (var.yres- 24) * 64;

    for (i = 0; i<4; i++) {
        max = 0;
        min = 10000;
        for (n = 0; n < wcslen(chinese_str[i]); n++) {
            // 设置变换，包括旋转和移动
            FT_Set_Transform(face, &matrix, &pen);

            // 将glyph加载到glyph插槽中，并将glyph转换为bitmap(位图)，保存在slot-bitmap中。
            error = FT_Load_Char(face, chinese_str[i][n], FT_LOAD_RENDER);
            if (error)
                continue;
            ///////////////////////////////////////////////////////////////////////
            // 从glyph插槽中获得glyph
            error = FT_Get_Glyph(face->glyph, &glyph);
            if (error) {
                printf("FT_Get_Glyph error!\n");
                return -1;
            }

            // 从glyph中获得bbox，bbox里就包含了metrics的相关信息
            FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox );
            if (bbox.yMin < min) {
                min = bbox.yMin;
            }
            if (bbox.yMax > max) {
                max = bbox.yMax;
            }
            // printf("origin.x/64 = %ld, origin.y/64 = %ld\n", pen.x/64, pen.y/64);
            // printf("slot->advance.x/64 = %ld, slot->advance.y/64 = %ld\n", slot->advance.x/64, slot->advance.y/64);
            draw_bitmap( &slot->bitmap, slot->bitmap_left, var.yres - slot->bitmap_top );

            // 下一个字
            pen.x += slot->advance.x;
            pen.y += slot->advance.y;
        }
        // printf("min = %d,max = %d\n", min, max);
        pen.x = var.xres / 2 * 64;
        pen.y = (var.yres - (max - min + 24*(i+1)) ) * 64;
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    printf("Press enter to exit\n");
    getchar();
}


