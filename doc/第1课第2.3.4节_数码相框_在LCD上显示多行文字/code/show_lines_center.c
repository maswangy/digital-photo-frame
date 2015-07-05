// gcc -finput-charset=GBK -fexec-charset=UTF-8 show_lines_center.c -o show_lines_center -lfreetype -lm

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

typedef struct  TGlyph_ {
    FT_UInt    index;  /* glyph index                  */
    FT_Vector  pos;    /* glyph origin on the baseline */
    FT_Glyph   image;  /* glyph image                  */

} TGlyph, *PGlyph;


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
        printf("Fail to support %dbpp\n", pixel_byte_size * 8);
    }
}

void draw_bitmap(FT_Bitmap*  bitmap, FT_Int x, FT_Int y)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;

    for (i = x, p = 0; i < x_max; i++, p++) {
        for (j = y, q = 0; j < y_max; j++, q++) {
            if (i < var.xres/2      || j < 0       ||
                    i >= var.xres || j >= var.yres)
                continue;
            lcd_draw_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
        }
    }
}

void Draw_Glyphs(PGlyph glyphs, FT_UInt num_glyphs, FT_Vector pos)
{
    int n;
    int error;

    for (n = 0; n < num_glyphs; n++) {

        // 将glyph转换为bitmap，同时再次设置位置。
        // 数组中的所有glyph原来的位置信息是以(0，0)为标准的，
        // 所以我们将所有glyph都偏移pos即可。
        // printf("x=%d, y=%d\n",pos.x, pos.y);
        error = FT_Glyph_To_Bitmap(&glyphs[n].image, FT_RENDER_MODE_NORMAL, &pos, 1 );

        if (!error)
        {
            FT_BitmapGlyph bit = (FT_BitmapGlyph)glyphs[n].image;
            draw_bitmap(&bit->bitmap, bit->left, var.yres - bit->top);
            FT_Done_Glyph(glyphs[n].image );
        }
    }
}


void  compute_string_bbox(FT_BBox  *abbox , int num_glyphs, PGlyph glyphs)
{
    int n;
    FT_BBox  bbox;
    bbox.xMin = bbox.yMin =  32000;
    bbox.xMax = bbox.yMax = -32000;

    for (n = 0; n < num_glyphs; n++) {
        FT_BBox  glyph_bbox;

        // 从glyph中获得bbox，FT_GLYPH_BBOX_TRUNCATE的意思是bbox里的单位是像素
        FT_Glyph_Get_CBox(glyphs[n].image, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);

        if (glyph_bbox.xMin < bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;

        if (glyph_bbox.yMin < bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;

        if (glyph_bbox.xMax > bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;

        if (glyph_bbox.yMax > bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;
    }

    if (bbox.xMin > bbox.xMax) {
        bbox.xMin = 0;
        bbox.yMin = 0;
        bbox.xMax = 0;
        bbox.yMax = 0;
    }
    *abbox = bbox;
}


// 保存glyph，返回glyph的个数
int Get_Glyph_From_Str(FT_Face face, wchar_t *chinese_str, PGlyph glyphs)
{
    PGlyph glyph_saver = glyphs;
    int n;
    FT_Error error;
    int pen_x = 0;
    int pen_y = 0;
    FT_GlyphSlot  slot = face->glyph;


    for (n = 0; n < wcslen(chinese_str); n++) {
        // 获得glyph索引
        glyph_saver->index = FT_Get_Char_Index(face, chinese_str[n]);

        /* store current pen position */
        glyph_saver->pos.x = pen_x;
        glyph_saver->pos.y = pen_y;

        // 根据索引从face中加载glyph到glyph插槽中
        error = FT_Load_Glyph(face, glyph_saver->index, FT_LOAD_DEFAULT);
        if ( error ) continue;

        // 从glyph插槽中获得glyph，并保存起来
        error = FT_Get_Glyph(face->glyph, &glyph_saver->image);
        if (error)
            continue;

        // printf("x=%d, y=%d\n",glyph_saver->pos.x, glyph_saver->pos.y);

        /* translate the glyph image now */
        // 设置glyph的位置
        FT_Glyph_Transform(glyph_saver->image, 0, &glyph_saver->pos);

        pen_x   += slot->advance.x;

        /* increment number of glyphs */
        glyph_saver++;
    }
    /* count number of glyphs loaded */
    return (glyph_saver - glyphs);
}


#define CHAR_HEIGHT 50
int main(int argc, char **argv)
{
    double angle;
    int n = 0;
    int i = 0;
    int max = 0;
    int min = 1000;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <font> \n", argv[0]);
        exit(1);
    }

    // open fb
    if ((fd_fb=open("/dev/fb0", O_RDWR)) == -1 ) {
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
    int line_box_width;
    int line_box_height;
    FT_Vector  pos;                       /* glyph origin on the baseline */

    // 用宽字符，一个字符用4个字节保存
    wchar_t *chinese_str[] = {L"吴伟东gif",L"嵌入式软件工程师",L"华南师范大学"};
    FT_Init_FreeType(&library);              /* initialize library */
    FT_New_Face(library, argv[1], 0, &face); /* create face object */
    FT_Set_Pixel_Sizes(face, CHAR_HEIGHT, 0);

#define MAX_GLYPHS 100
    TGlyph        glyphs[MAX_GLYPHS];  /* glyphs table */
    FT_UInt       num_glyphs;
    FT_BBox       frame;

    for (i = 0; i < 3; i++) {
        // 保存glyph
        num_glyphs = Get_Glyph_From_Str(face, chinese_str[i], glyphs);

        // 获得边框
        compute_string_bbox(&frame, num_glyphs, glyphs);
        line_box_width  = frame.xMax - frame.xMin;
        line_box_height = frame.yMax - frame.yMin;

        // 描绘字符
        pos.x = (var.xres/2 + (var.xres/2 - line_box_width) / 2) * 64;
        pos.y = ((var.yres - line_box_height)/2 - CHAR_HEIGHT*i ) * 64;

        // printf("start:x=%d, y=%d\n",pos.x, pos.y);
        Draw_Glyphs(glyphs, num_glyphs, pos);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}



