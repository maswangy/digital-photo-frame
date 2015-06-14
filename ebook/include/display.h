/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : display.h
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "config.h"
#include "list.h"

struct display_ops {
    struct list_head list;
    char *name;
    int type;
    int xres;
    int yres;
    int (*init)();
    int (*draw_pixel)(int x, int y, unsigned int color);
};

int register_display_ops(struct display_ops *ops);
int deregister_display_ops(struct display_ops *ops);
int display_init(void);
int display_exit(void);
int display_select(struct txt_info *txt);
void display_list(void);

int fb_display_init(void);
int fb_display_exit(void);


#endif /* __DISPLAY_H_ */
