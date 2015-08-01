/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ascii_8x8.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "bitmap.h"
#include "8x8.h"

static int ascii_8x8_init(char *txt_path, int bitmap_size)
{
    return 0;
}

static int ascii_8x8_is_supported(int encode)
{
    if (encode == ENCODE_ASCII) {
        return 1;
    }
    return 0;
}

static int ascii_8x8_get_char_bitmap(unsigned int code, unsigned char **bitmap, struct bitmap_info *bif)
{
    if (code > 0x80) {
        return -1;
    }
    struct cell_frame *cf = &(bif->cf);
    struct font_frame *ff = &(bif->ff);

    *bitmap = (unsigned char *)&fontdata_8x8[code*8];

    ff->xmin = cf->xmin;
    ff->ymin = cf->ymin;
    ff->xmax = ff->xmin + 8;
    ff->ymax = ff->ymin + 8;
    ff->width = 8;
    ff->height = 8;
    
    return 0;
}

static struct bitmap_ops ascii_8x8_bitmap_ops = {
        .name = "ascii_8x8",
        .type = BITMAP_ASCII_8X8,
        .bpp  = 1,
        .init = ascii_8x8_init,
        .is_supported = ascii_8x8_is_supported,
        .get_char_bitmap = ascii_8x8_get_char_bitmap,
};

int ascii_8x8_bitmap_init(void)
{
    if (register_bitmap_ops(&ascii_8x8_bitmap_ops) == -1) {
        ERR("fail to register %s bitmap ops\n", ascii_8x8_bitmap_ops.name);
        return -1;
    }
    return 0;
}

int ascii_8x8_bitmap_exit(void)
{
    if (deregister_bitmap_ops(&ascii_8x8_bitmap_ops) == -1) {
        ERR("fail to deregister %s bitmap ops\n", ascii_8x8_bitmap_ops.name);
        return -1;
    }
    return 0;
}
