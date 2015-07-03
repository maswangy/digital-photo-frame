/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : bitmap.h
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#ifndef __BITMAP_H_
#define __BITMAP_H_

#include "config.h"
#include "list.h"

struct cell_frame {
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    int width;
    int height;
};

struct font_frame {
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    int width;
    int height;
};

struct bitmap_info {
    struct cell_frame cf;
    struct font_frame ff;
};

struct bitmap_ops {
    struct list_head list;
    char *name;
    int type;
    int bpp;
    int (*init)(char *txt_path, int bitmap_size);
    int (*is_supported)(int encode);
    int (*get_char_bitmap)(unsigned int code, unsigned char **bitmap, struct bitmap_info *bif);
};

int register_bitmap_ops(struct bitmap_ops *ops);
int deregister_bitmap_ops(struct bitmap_ops *ops);
int bitmap_init(void);
int bitmap_exit(void);
int bitmap_select(struct txt_info *txt);
void bitmap_list(void);

#if 0
int freetype_bitmap_init(void);
int freetype_bitmap_exit(void);
#endif
int ascii_8x8_bitmap_init(void);
int ascii_8x8_bitmap_exit(void);
int ascii_8x16_bitmap_init(void);
int ascii_8x16_bitmap_exit(void);
int hzk_bitmap_init(void);
int hzk_bitmap_exit(void);
int freetype_bitmap_init(void);
int freetype_bitmap_exit(void);

#endif /* __BITMAP_H_ */
