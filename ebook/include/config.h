/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : config.h
** Author            : wuweidong  
** Last modified data: 2015-6-9
** Description       :
** Todo              :
********************************************************************************/

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "list.h"

struct page {
    struct list_head list;
    int id;
    unsigned char *buf;
};

struct txt_info {
    int fd;
    char ttc_path[64];
    unsigned char *buf;
    int length;
    struct encode_ops *ecd_ops;
    struct bitmap_ops *bmp_ops;
    struct display_ops *dsp_ops;
};

#define ENCODE_ASCII        (1)
#define ENCODE_UTF8         (2)
#define ENCODE_UTF16BE      (3)
#define ENCODE_UTF16LE      (4)
#define ENCODE_ISO8859      (5)

#define BITMAP_FREETYPE     (1)
#define BITMAP_HZK          (2)
#define BITMAP_ASCII_8X8    (3)
#define BITMAP_ASCII_8X16   (4)

#define DISPLAY_FB          (1)

#define PRINT_ERR printf
#define PRINT_DBG printf
#define PRINT_INFO printf
// #define PRINT_ERR(...)
// #define PRINT_DBG(...)
// #define PRINT_INFO(...)

#endif /* __CONFIG_H_ */
