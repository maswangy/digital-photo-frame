/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : iso-8859.c
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#include "config.h"
#include "encode.h"

static int iso8859_is_supported(const unsigned char *buf, int length)
{
    if (buf == NULL || length < 0) {
        return 0;
    }

    int i = 0;
    for (i = 0; i < length; i++) {
        int t = text_chars[buf[i]];
        if (t != T && t != I) {
            // DBG("Not iso8859 code:%x\n", buf[i]);
            return 0;
        }
    }
    return 1;
}

static int iso8859_get_char_code(const unsigned char *buf, unsigned int *code)
{
    if (buf == NULL || code == NULL || (text_chars[*buf] != T && text_chars[*buf] != I)) {
        return -1;
    }
    if (buf[0] < (unsigned char)0x80) {
        // handle windows enter code
        if ( buf[0] == 0xd && buf[1] == 0xa) {
            *code = (buf[0]<<8) | buf[1];
            return 2;
        } else {
            *code = buf[0];
            return 1;
        }
    } else {
        *code = (buf[0]<<8) | buf[1];
        return 2;
    }
}

static struct encode_ops iso8859_encode_ops = {
        .name = "iso8859",
        .type = ENCODE_ISO8859,
        .is_supported = iso8859_is_supported,
        .get_char_code = iso8859_get_char_code,
};

int iso8859_encode_init(void)
{
    if (register_encode_ops(&iso8859_encode_ops) == -1) {
        ERR("fail to register %s encode ops\n", iso8859_encode_ops.name);
        return -1;
    }
    return 0;
}

int iso8859_encode_exit(void)
{
    if (deregister_encode_ops(&iso8859_encode_ops) == -1) {
        ERR("fail to deregister %s encode ops\n", iso8859_encode_ops.name);
        return -1;
    }
    return 0;
}
