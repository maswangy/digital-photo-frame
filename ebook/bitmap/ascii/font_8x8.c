/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : font_8x8.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "bitmap.h"

int font_8x8_get_char_bitmap(const unsigned char *buf)
{
    return 0;
}
struct bitmap_ops font_8x8_bitmap_ops = {
        .name = "font_8x8",
        .type = BITMAP_ASCII_8X8,
        .get_char_bitmap = font_8x8_get_char_bitmap,
};

int font_8x8_bitmap_init(void)
{
    if (register_bitmap_ops(&font_8x8_bitmap_ops) == -1) {
        PRINT_ERR("fail to register %s bitmap ops\n", font_8x8_bitmap_ops.name);
        return -1;
    }
    return 0;
}

int font_8x8_bitmap_exit(void)
{
    if (deregister_bitmap_ops(&font_8x8_bitmap_ops) == -1) {
        PRINT_ERR("fail to deregister %s bitmap ops\n", font_8x8_bitmap_ops.name);
        return -1;
    }
    return 0;
}
