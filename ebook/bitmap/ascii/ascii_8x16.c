/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ascii_8x16.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "bitmap.h"
#include "8x16.h"

static int ascii_8x16_init(void)
{
    return 0;
}

static int ascii_8x16_is_supported(int encode)
{
    if (encode == ENCODE_ASCII) {
        return 1;
    }
    return 0;
}

static int ascii_8x16_get_char_bitmap(unsigned int code, unsigned char **bitmap, struct char_frame *cf)
{
    if (code > 0x80) {
        return -1;
    }
    *bitmap = (unsigned char *)&fontdata_8x16[code*16];    
    cf->xmax = cf->xmin + 8;
    cf->ymax = cf->ymin + 16;
    cf->width = 8;
    cf->height = 16;
    
    return 0;
}

static struct bitmap_ops ascii_8x16_bitmap_ops = {
        .name = "ascii_8x16",
        .type = BITMAP_ASCII_8X16,
        .init = ascii_8x16_init,
        .is_supported = ascii_8x16_is_supported,
        .get_char_bitmap = ascii_8x16_get_char_bitmap,
};

int ascii_8x16_bitmap_init(void)
{
    if (register_bitmap_ops(&ascii_8x16_bitmap_ops) == -1) {
        PRINT_ERR("fail to register %s bitmap ops\n", ascii_8x16_bitmap_ops.name);
        return -1;
    }
    return 0;
}

int ascii_8x16_bitmap_exit(void)
{
    if (deregister_bitmap_ops(&ascii_8x16_bitmap_ops) == -1) {
        PRINT_ERR("fail to deregister %s bitmap ops\n", ascii_8x16_bitmap_ops.name);
        return -1;
    }
    return 0;
}
