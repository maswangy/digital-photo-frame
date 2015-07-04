/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : freetype.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include <math.h>
#include "config.h"
#include "bitmap.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

static FT_GlyphSlot  slot;
static FT_Library    library;
static FT_Face       face;
static FT_Matrix     matrix;

static int freetype_init(char *ttc_path, int bitmap_size)
{
    int size = 24;
    double angle = ( 0.0 / 360 ) * 3.14159 * 2;

    if (bitmap_size != 0) {
        size = bitmap_size;
    }
    if (FT_Init_FreeType(&library)) {
        return -1;
    }
    if (FT_New_Face(library, ttc_path, 0, &face)) {
        return -1;
    }
    if (FT_Set_Pixel_Sizes(face, size, 0)) {
        return -1;
    }
    slot = face->glyph;
    /* set up matrix */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
    return 0;
}

static int freetype_is_supported(int encode)
{
    if (encode == ENCODE_ASCII || encode == ENCODE_UTF16BE || encode == ENCODE_UTF16LE || encode == ENCODE_UTF8) {
        return 1;
    }
    return 0;
}

static int freetype_get_char_bitmap(unsigned int code, unsigned char **bitmap, struct bitmap_info *bif)
{
    FT_Vector pen;
    FT_Bitmap *ft_bitmap;
    FT_BBox bbox;
    FT_Glyph  glyph;

    struct cell_frame *cf = &(bif->cf);
    struct font_frame *ff = &(bif->ff);

    pen.x = cf->xmin * 64;
    pen.y = cf->ymin * 64;

    FT_Set_Transform(face, &matrix, &pen);

    if (FT_Load_Char(face, code, FT_LOAD_RENDER)) {
        PRINT_DBG("fail to FT_Get_Glyph\n");
        return -1;
    }
    if (FT_Get_Glyph(face->glyph, &glyph)) {
        PRINT_DBG("fail to FT_Get_Glyph\n");
        return -1;
    }

    FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox );
    if (bbox.xMin == 0 && bbox.xMax == 0 && bbox.yMin == 0 && bbox.yMax == 0) {
        return 0;
    }
    ft_bitmap = &slot->bitmap;
    *bitmap = ft_bitmap->buffer;
    ff->xmin = bbox.xMin;
    ff->xmax = bbox.xMin + ft_bitmap->width;
    ff->ymin = bbox.yMin;
    ff->ymax = bbox.yMin + ft_bitmap->rows;
    ff->width = ft_bitmap->width;
    ff->height = ft_bitmap->rows;
#if 0
    PRINT_DBG("\npen.x = %d, pen.y = %d, xMin = %d, xMax = %d, yMin = %d, yMax = %d\n", \
            pen.x / 64, pen.y / 64, \
            ff->xmin, ff->xmax, ff->ymin, ff->ymax);
    PRINT_DBG("width=%d, height=%d\n", ff->width, ff->height);
#endif    
    return 0;
}

static struct bitmap_ops freetype_bitmap_ops = {
        .name = "freetype",
        .type = BITMAP_FREETYPE,
        .bpp  = 8,
        .init = freetype_init,
        .is_supported = freetype_is_supported,
        .get_char_bitmap = freetype_get_char_bitmap,
};

int freetype_bitmap_init(void)
{
    if (register_bitmap_ops(&freetype_bitmap_ops) == -1) {
        PRINT_ERR("fail to register %s bitmap ops\n", freetype_bitmap_ops.name);
        return -1;
    }
    return 0;
}

int freetype_bitmap_exit(void)
{
    if (deregister_bitmap_ops(&freetype_bitmap_ops) == -1) {
        PRINT_ERR("fail to deregister %s bitmap ops\n", freetype_bitmap_ops.name);
        return -1;
    }
    return 0;
}
