/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : encode.c
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#include "encode.h"

static struct list_head entry;

int register_encode_ops(struct encode_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_add(&(ops->list), &entry);
    return 0;
}

int deregister_encode_ops(struct encode_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_del(&(ops->list));
    return 0;
}

void encode_list(void)
{
    struct list_head *list;

    PRINT_INFO("registered encode:\n");
    list_for_each(list, &entry) {
        struct encode_ops *ops = list_entry(list, struct encode_ops, list);
        PRINT_INFO("%s\n", ops->name);
    }
}

int encode_select(struct txt_info *txt)
{
    struct list_head *list;

    list_for_each(list, &entry) {
        struct encode_ops *ops = list_entry(list, struct encode_ops, list);
        if (ops->is_supported(txt->buf, txt->length)) {
            txt->ecd_ops = ops;
            PRINT_INFO("selected encode:\n%s\n", ops->name);
            return 0;
        }
    }
    return -1;
}

int encode_init(void)
{
    INIT_LIST_HEAD(&entry);

    if (ascii_encode_init() == -1) {
        PRINT_ERR("fail to init ascii encode\n");
        return -1;
    }

#if 0
    if (iso8859_encode_init() == -1) {
        PRINT_ERR("fail to init iso8859 encode\n");
        return -1;
    }
#endif
    return 0;
}

int encode_exit(void)
{
#if 0
    if (iso8859_encode_exit() == -1) {
        PRINT_ERR("fail to exit iso8859 encode\n");
        return -1;
    }
#endif
    if (ascii_encode_exit() == -1) {
        PRINT_ERR("fail to exit ascii encode\n");
        return -1;
    }
    return 0;
}

