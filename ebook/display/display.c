/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : display.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "display.h"

static struct list_head entry;

int register_display_ops(struct display_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_add(&(ops->list), &entry);
    return 0;
}

int deregister_display_ops(struct display_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_del(&(ops->list));
    return 0;
}

void display_list(void)
{
    struct list_head *list;

    PRINT_INFO("registered display:\n");
    list_for_each(list, &entry) {
        struct display_ops *ops = list_entry(list, struct display_ops, list);
        PRINT_INFO("%s\n", ops->name);
    }
    PRINT_INFO("\n");
}

int display_select(struct txt_info *txt)
{
    PRINT_DBG("display_select\n");
    struct list_head *list;

    list_for_each(list, &entry) {
        struct display_ops *ops = list_entry(list, struct display_ops, list);
        // only support framebuffer now
        if (ops->type == DISPLAY_FB) {
            txt->dsp_ops = ops;
            PRINT_INFO("selected display:\n%s\n", ops->name);
            return 0;
        }
    }
    return -1;
}

int display_init(void)
{
    INIT_LIST_HEAD(&entry);

    if (fb_display_init() == -1) {
        PRINT_ERR("fail to init framebuffer display\n");
        return -1;
    }
    return 0;
}

int display_exit(void)
{
    if (fb_display_exit() == -1) {
        PRINT_ERR("fail to exit framebuffer display\n");
        return -1;
    }

    return 0;
}
