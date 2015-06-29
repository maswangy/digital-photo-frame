/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : freetype.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "bitmap.h"

static int freetype_init(void)
{
    return 0;
}

static int freetype_is_supported(int encode)
{
    PRINT_DBG("freetype_is_supported %d\n", encode);
    if (encode == ENCODE_ASCII || encode == ENCODE_UTF16BE || encode == ENCODE_UTF16LE || encode == ENCODE_UTF8) {
        return 1;
    }
    return 0;
}

static int freetype_get_char_bitmap(unsigned int code, unsigned char **bitmap, struct char_frame *cf)
{
    return 0;
}

static struct bitmap_ops freetype_bitmap_ops = {
        .name = "freetype",
        .type = BITMAP_FREETYPE,
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
