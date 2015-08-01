/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : utf16_be.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "encode.h"

static int utf16_be_is_supported(const unsigned char *buf, int length)
{
    if (buf == NULL || length < 0) {
        return 0;
    }

    // only work for file with BOM(Byte Order Mark)
    if (length>=2 && buf[0]==0xfe && buf[1]==0xff)
        return 1;
    return 0;
}

static int utf16_be_get_char_code(const unsigned char *buf, unsigned int *code)
{
    if (buf == NULL || code == NULL) {
        return -1;
    }
    
    *code = buf[0]<<8 | buf[1];
    return 2;

}

static struct encode_ops utf16_be_encode_ops = {
        .name = "utf16_be",
        .type = ENCODE_UTF16BE,
        .is_supported = utf16_be_is_supported,
        .get_char_code = utf16_be_get_char_code,
};

int utf16_be_encode_init(void)
{
    if (register_encode_ops(&utf16_be_encode_ops) == -1) {
        ERR("fail to register %s encode ops\n", utf16_be_encode_ops.name);
        return -1;
    }
    return 0;
}

int utf16_be_encode_exit(void)
{
    if (deregister_encode_ops(&utf16_be_encode_ops) == -1) {
        ERR("fail to deregister %s encode ops\n", utf16_be_encode_ops.name);
        return -1;
    }
    return 0;
}
