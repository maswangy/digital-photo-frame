/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : hzk.c
 ** Author            : wuweidong  
 ** Description       :
 ** Todo              :
 ********************************************************************************/
#include "config.h"
#include "bitmap.h"
#include "8x16.h"

unsigned char *hzk_mem;

static int hzk_init(void)
{
    int fd_hzk;
    struct stat hzk_stat;
    
    // open HZK
    if ((fd_hzk = open("HZK16", O_RDONLY)) == -1) {
        PRINT_ERR("fail to open HZK16\n");
        return -1;
    }
    // get HZK stat
    if (fstat(fd_hzk, &hzk_stat) == -1) {
        PRINT_ERR("fail to fstat fd_hzk\n");
        return -1;
    }
    // map HZK
    if ((hzk_mem = mmap(NULL, hzk_stat.st_size, PROT_READ, MAP_SHARED, fd_hzk, 0)) == MAP_FAILED) {
        PRINT_ERR("fail to mmap HZK16\n");
        return -1;
    }

    return 0;
}

static int hzk_is_supported(int encode)
{
    if (encode == ENCODE_ASCII || encode == ENCODE_ISO8859) {
        return 1;
    }
    return 0;
}

static int hzk_get_char_bitmap(unsigned int code, unsigned char **bitmap, struct char_frame *cf)
{    
    int areacode;
    int bitcode;
    
    if (code <= 0x80) {
        // use ascii_8x16 bitmap
        *bitmap = (unsigned char *)&fontdata_8x16[code*16];    
        cf->xmax = cf->xmin + 8;
        cf->ymax = cf->ymin + 16;
        cf->width = 8;
        cf->height = 16;
    } else {
        // use hzk16 bitmap
        areacode = (code>>8 & 0xff) - 0xA1;
        bitcode = (code & 0xff) - 0xA1;
        *bitmap = hzk_mem + areacode * 94 * 32 + bitcode * 32;
        cf->xmax = cf->xmin + 16;
        cf->ymax = cf->ymin + 16;
        cf->width = cf->height = 16;
    }    
    return 0;
}

static struct bitmap_ops hzk_bitmap_ops = {
        .name = "hzk",
        .type = BITMAP_HZK,
        .init = hzk_init,
        .is_supported = hzk_is_supported,
        .get_char_bitmap = hzk_get_char_bitmap,
};

int hzk_bitmap_init(void)
{
    if (register_bitmap_ops(&hzk_bitmap_ops) == -1) {
        PRINT_ERR("fail to register %s bitmap ops\n", hzk_bitmap_ops.name);
        return -1;
    }
    return 0;
}

int hzk_bitmap_exit(void)
{
    if (deregister_bitmap_ops(&hzk_bitmap_ops) == -1) {
        PRINT_ERR("fail to deregister %s bitmap ops\n", hzk_bitmap_ops.name);
        return -1;
    }
    return 0;
}