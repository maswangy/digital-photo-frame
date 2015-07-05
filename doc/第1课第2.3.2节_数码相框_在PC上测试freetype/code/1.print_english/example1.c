/* example1.c                                                      */
/*                                                                 */
/* This small program shows how to print a rotated string with the */
/* FreeType 2 library.                                             */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#define WIDTH   80
#define HEIGHT  80

/* origin is the upper left corner */
unsigned char image[HEIGHT][WIDTH];

/* Replace this function with something useful. */
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
                    i >= WIDTH || j >= HEIGHT)
                continue;
            image[j][i] |= bitmap->buffer[q * bitmap->width + p];
        }
    }
}

void show_image(void)
{
    int  i, j;

    for (i = 0; i < HEIGHT; i++) {
        printf("%d", i);
        for (j = 0; j < WIDTH; j++) {
            putchar(image[i][j] == 0 ? ' '
                    : image[i][j] < 128 ? '+'
                            : '*');
        }
        putchar('\n');
    }
}


int main(int argc, char**  argv)
{
    // ��
    FT_Library    library;
    // �����ļ�
    FT_Face       face;
    // glyph���
    FT_GlyphSlot  slot;
    // ��ת����
    FT_Matrix     matrix;                 /* transformation matrix */
    // �ƶ�����
    FT_Vector     pen;                    /* untransformed origin  */
    FT_Error      error;

    char*         filename;
    char*         text;

    double        angle;
    int           target_height;
    int           n, num_chars;

    // ./example1 simsun.tcc abc
    if (argc != 3) {
        fprintf ( stderr, "usage: %s font sample-text\n", argv[0] );
        exit(1);
    }

    filename      = argv[1];                           /* first argument     */
    text          = argv[2];                           /* second argument    */
    num_chars     = strlen(text);

#if 0
    angle         = (25.0 / 360) * 3.14159 * 2;      /* use 25 degrees     */
#else
    angle         = (0 / 360) * 3.14159 * 2;         // ��ת0��
#endif

    target_height = HEIGHT;

    // ��ʼ��freetype��
    error = FT_Init_FreeType(&library);              /* initialize library */
    /* error handling omitted */


    // �������ļ�
    error = FT_New_Face(library, argv[1], 0, &face); /* create face object */
    /* error handling omitted */

#if 0
    /* use 50pt at 100dpi */
    error = FT_Set_Char_Size( face, 50 * 64, 0,
            100, 0 );                /* set character size */
    /* error handling omitted */
#else
    error = FT_Set_Pixel_Sizes(face, 24, 0);			// �ַ��ĸ� = �� = 24 ����
    /* error handling omitted */
#endif

    slot = face->glyph;

    // ������ת����
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

    // ����λ������:
    // �ַ�����λ�ã���ΪLCD������ϵ��Freetype������ϵ��ͬ��
    // ��LCD(20,20)����Ҳ����Freetype��(20,LCD�߶�-20)����
    // pen.x��pen.y�ĵ�λ��1/64����
    pen.x = 20 * 64;
    pen.y = (target_height - 20) * 64;


    // �����ӡ�ַ�
    for (n = 0; n < num_chars; n++) {
        // ���ñ任��������ת��λ��
        FT_Set_Transform(face, &matrix, &pen);

        // �������ļ��м����ַ���glyph��Ȼ���glyphת��Ϊbitmap��������slot->bitmap�У�
        // bitmap������
        error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);
        if (error)
            continue;                 /* ignore errors */

        // slot->bitmap�ǵ���
        // slot->bitmap_left��LCD�ϵ�x����
        // target_height - slot->bitmap_top��LCD�ϵ�y����
        draw_bitmap(&slot->bitmap, slot->bitmap_left, target_height - slot->bitmap_top);

        /* increment pen position */
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;

    }

    show_image();

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    return 0;
}

/* EOF */
