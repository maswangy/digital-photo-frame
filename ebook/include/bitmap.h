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

struct bitmap_ops {
    struct list_head list;
    char *name;
    int type;
    int (*get_char_bitmap)(const unsigned char *buf);
};

int register_bitmap_ops(struct bitmap_ops *ops);
int deregister_bitmap_ops(struct bitmap_ops *ops);
int bitmap_init(void);
int bitmap_exit(void);
#if 0
int bitmap_select(struct txt_info *txt);
void bitmap_list(void);
#endif

#if 0
int freetype_bitmap_init(void);
int freetype_bitmap_exit(void);
#endif
int font_8x8_bitmap_init(void);
int font_8x8_bitmap_exit(void);




#endif /* __BITMAP_H_ */
