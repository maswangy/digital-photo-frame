/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ascii.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "encode.h"


static char text_chars[256] = {
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

static int ascii_is_supported(const unsigned char *buf, int length)
{
    if (buf == NULL || length < 0) {
        return 0;
    }

    int i = 0;
    for (i = 0; i < length; i++) {
        int t = text_chars[buf[i]];
        if (t != T)
            return 0;
    }
    return 1;
}

static int ascii_get_char_code(const unsigned char *buf, unsigned int *code)
{
    if (buf == NULL || code == NULL || text_chars[*buf] != T) {
        return -1;
    }
    *code = (unsigned int)(*buf);
    return 1;
}

struct encode_ops ascii_encode_ops = {
        .name = "ascii",
        .type = ENCODE_ASCII,
        .is_supported = ascii_is_supported,
        .get_char_code = ascii_get_char_code,
};

int ascii_encode_init(void)
{
    if (register_encode_ops(&ascii_encode_ops) == -1) {
        PRINT_ERR("fail to register %s encode ops\n", ascii_encode_ops.name);
        return -1;
    }
    return 0;
}

int ascii_encode_exit(void)
{
    if (deregister_encode_ops(&ascii_encode_ops) == -1) {
        PRINT_ERR("fail to deregister %s encode ops\n", ascii_encode_ops.name);
        return -1;
    }
    return 0;
}
