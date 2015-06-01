/* example1.c                                                      */
/*                                                                 */
/* This small program shows how to print a rotated string with the */
/* FreeType 2 library.                                             */


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wchar.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

static unsigned char *fb_mem;
static int screen_byte_size;
static int line_byte_size;
static int pixel_byte_size;
static struct fb_var_screeninfo var;
static struct fb_fix_screeninfo fix;

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
    default: {
        printf("fail to suppor %d bpp\n",pixel_byte_size * 8);
    }
    }
}

void draw_bitmap(FT_Bitmap*  bitmap,
        FT_Int      x,
        FT_Int      y)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;


    for (i = x, p = 0; i < x_max; i++, p++) {
        for (j = y, q = 0; j < y_max; j++, q++) {
            if (i < 0      || j < 0       ||
                    i >= var.xres || j >= var.yres)
                continue;
            // printf("%d * %d + %d = %d\n", q, bitmap->width, p, bitmap->buffer[q * bitmap->width + p]);
            if (bitmap->buffer[q * bitmap->width + p] != 0) {
                lcd_draw_pixel(i, j, 0x0000ff);           // blue
            } else {
                lcd_draw_pixel(i, j, 0xffffff);           // black
            }
        }
    }
}

int fb_init(void)
{
    int fd_fb = -1;
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
    return 0;
}

int main(int     argc,
        char**  argv)
{
    FT_Library    library;
    FT_Face       face;

    FT_GlyphSlot  slot;
    FT_Matrix     matrix;                 /* transformation matrix */
    FT_Vector     pen;                    /* untransformed origin  */
    FT_Error      error;

    char*         filename;
    char*         text;

    double        angle;
    int           target_height;
    int           n, num_chars;

    // 不能用下面这种方式，因为汉字用两个字节，而字母用一个字节，如果用下面
    // 这种方式的话，需要自己分析何时是汉字，何时是字母。
    //char *str = "吴伟东ABC";

    // 用宽字符，一个字符用4个字节保存
    wchar_t *chinese_str = L"吴伟东ABC";
    unsigned int *p = (wchar_t *)chinese_str;
    int i;

    printf("Uniocde: \n");
    for (i = 0; i < wcslen(chinese_str); i++) {
        printf("0x%x ", p[i]);
    }
    printf("\n");

    if (argc != 2) {
        fprintf(stderr, "usage: %s font\n", argv[0]);
        exit(1);
    }

    fb_init();

    filename      = argv[1];                           /* first argument     */
    angle         = (0.0 / 360) * 3.14159 * 2;      /* use 25 degrees     */
    target_height = var.yres;

    error = FT_Init_FreeType( &library );              /* initialize library */
    /* error handling omitted */

    error = FT_New_Face( library, argv[1], 0, &face ); /* create face object */
    /* error handling omitted */

#if 0
    /* use 50pt at 100dpi */
    error = FT_Set_Char_Size( face, 50 * 64, 0,
            100, 0 );                /* set character size */

    /* pixels = 50 /72 * 100 = 69  */
#else
    FT_Set_Pixel_Sizes(face, 24, 0);
#endif
    /* error handling omitted */

    slot = face->glyph;

    /* set up matrix */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

    /* the pen position in 26.6 cartesian space coordinates; */
    /* start at (0,40) relative to the upper left corner  */
    pen.x = var.xres/2 * 64;
    pen.y = ( target_height - var.yres/2 ) * 64;

    for (n = 0; n < wcslen(chinese_str); n++) {
        /* set transformation */
        FT_Set_Transform(face, &matrix, &pen);

        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char(face, chinese_str[n], FT_LOAD_RENDER);
        if (error)
            continue;                 /* ignore errors */
        /* now, draw to our target surface (convert position) */
        draw_bitmap( &slot->bitmap,
                slot->bitmap_left,
                target_height - slot->bitmap_top );

        /* increment pen position */
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }

    FT_Done_Face    (face);
    FT_Done_FreeType(library);

    return 0;
}

/* EOF */
