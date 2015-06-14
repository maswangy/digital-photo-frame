/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : bitmap.c
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#include "bitmap.h"

static struct list_head entry;

int register_bitmap_ops(struct bitmap_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_add(&(ops->list), &entry);
    return 0;
}

int deregister_bitmap_ops(struct bitmap_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_del(&(ops->list));
    return 0;
}

#if 0
void bitmap_list(void)
{
    struct list_head *list;

    PRINT_INFO("registered bitmap:\n");
    list_for_each(list, &entry) {
        struct bitmap_ops *ops = list_entry(list, struct bitmap_ops, list);
        PRINT_INFO("%s\n", ops->name);
    }
}

int bitmap_select(struct txt_info *txt)
{
    struct list_head *list;

    list_for_each(list, &entry) {
        struct bitmap_ops *ops = list_entry(list, struct bitmap_ops, list);
        if (ops->is_supported(txt->buf, txt->length)) {
            txt->bitmap = ops->type;
            PRINT_INFO("selected bitmap:%s\n", ops->name);
            return 0;
        }
    }
    return -1;
}
#endif

int bitmap_init(void)
{
    INIT_LIST_HEAD(&entry);

#if 0
    if (freetype_bitmap_init() == -1) {
        PRINT_ERR("fail to init freetype bitmap\n");
        return -1;
    }
#endif
    if (font_8x8_bitmap_init() == -1) {
        PRINT_ERR("fail to init font 8x8 bitmap\n");
        return -1;
    }
    return 0;
}

int bitmap_exit(void)
{
#if 0
    if (freetype_bitmap_exit() == -1) {
        PRINT_ERR("fail to exit freetype bitmap\n");
        return -1;
    }
#endif
    if (font_8x8_bitmap_exit() == -1) {
        PRINT_ERR("fail to exit font 8x8 bitmap\n");
        return -1;
    }

    return 0;
}

