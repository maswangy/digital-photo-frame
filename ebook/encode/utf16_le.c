/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : utf16_le.c
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#include "config.h"
#include "encode.h"

static int utf16_le_is_supported(const unsigned char *buf, int length)
{
    if (buf == NULL || length < 0) {
        return 0;
    }

    // only work for file with BOM(Byte Order Mark)
    if (length>=2 && buf[0]==0xff && buf[1]==0xfe)
        return 1;
    return 0;
}

static int utf16_le_get_char_code(const unsigned char *buf, unsigned int *code)
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

static struct encode_ops utf16_le_encode_ops = {
        .name = "utf16_le",
        .type = ENCODE_UTF16LE,
        .is_supported = utf16_le_is_supported,
        .get_char_code = utf16_le_get_char_code,
};

int utf16_le_encode_init(void)
{
    if (register_encode_ops(&utf16_le_encode_ops) == -1) {
        PRINT_ERR("fail to register %s encode ops\n", utf16_le_encode_ops.name);
        return -1;
    }
    return 0;
}

int utf16_le_encode_exit(void)
{
    if (deregister_encode_ops(&utf16_le_encode_ops) == -1) {
        PRINT_ERR("fail to deregister %s encode ops\n", utf16_le_encode_ops.name);
        return -1;
    }
    return 0;
}
