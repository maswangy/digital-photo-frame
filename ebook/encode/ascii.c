/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ascii.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "encode.h"

static int ascii_is_supported(const unsigned char *buf, int length)
{
    if (buf == NULL || length < 0) {
        return 0;
    }

    int i = 0;
    for (i = 0; i < length; i++) {
        int t = text_chars[buf[i]];
        if (t != T) {
            // PRINT_DBG("Not ascii code:%x\n", buf[i]);
            return 0;
        }
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

static struct encode_ops ascii_encode_ops = {
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
