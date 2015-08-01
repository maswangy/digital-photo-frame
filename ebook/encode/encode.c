/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : encode.c
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#include "encode.h"

static struct list_head entry;
char text_chars[256] = {
    /*                  BEL BS HT LF    FF CR    */
    F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
    /*                              ESC          */
    F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
    T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
    /*            NEL                            */
    X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
    X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
};

int register_encode_ops(struct encode_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_add_tail(&(ops->list), &entry);
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
    int i = 1;

    INFO("registered encode:\n");
    list_for_each(list, &entry) {
        struct encode_ops *ops = list_entry(list, struct encode_ops, list);
        INFO("%d.%s\n", i, ops->name);
        i++;
    }
    INFO("\n");
}

int encode_select(struct txt_info *txt)
{
    struct list_head *list;

    list_for_each(list, &entry) {
        struct encode_ops *ops = list_entry(list, struct encode_ops, list);
        if (ops->is_supported(txt->start, txt->length)) {
            txt->ecd_ops = ops;
            INFO("selected encode:\n%s\n\n", ops->name);
            return 0;
        }
    }
    return -1;
}

int encode_init(void)
{
    INIT_LIST_HEAD(&entry);

    if (utf16_le_encode_init() == -1) {
        ERR("fail to init utf16_le encode\n");
        return -1;
    }

    if (utf16_be_encode_init() == -1) {
        ERR("fail to init utf16_be encode\n");
        return -1;
    }

    if (utf8_encode_init() == -1) {
        ERR("fail to init utf8 encode\n");
        return -1;
    }

    if (iso8859_encode_init() == -1) {
        ERR("fail to init iso8859 encode\n");
        return -1;
    }

    if (ascii_encode_init() == -1) {
        ERR("fail to init ascii encode\n");
        return -1;
    }

    return 0;
}

int encode_exit(void)
{
    if (ascii_encode_exit() == -1) {
        ERR("fail to exit ascii encode\n");
        return -1;
    }

    if (iso8859_encode_exit() == -1) {
        ERR("fail to exit iso8859 encode\n");
        return -1;
    }

    if (utf8_encode_exit() == -1) {
        ERR("fail to exit utf8 encode\n");
        return -1;
    }

    if (utf16_be_encode_exit() == -1) {
        ERR("fail to exit utf16_be encode\n");
        return -1;
    }

    if (utf16_le_encode_exit() == -1) {
        ERR("fail to exit utf16_le encode\n");
        return -1;
    }
    return 0;
}

