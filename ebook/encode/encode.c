/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : encode.c
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#include "encode.h"

struct list_head entry;

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
            txt->encode = ops->type;
            PRINT_INFO("selected encode:%s\n", ops->name);
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
    return 0;
}

int encode_exit(void)
{
    if (ascii_encode_exit() == -1) {
        PRINT_ERR("fail to exit ascii encode\n");
        return -1;
    }
    return 0;
}

