/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : freetype.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "bitmap.h"

int freetype_get_char_bitmap(const unsigned char *buf)
{
    return 0;
}
struct bitmap_ops freetype_bitmap_ops = {
        .name = "freetype",
        .type = BITMAP_FREETYPE,
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
