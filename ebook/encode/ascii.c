/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ascii.c
 ** Author            : wuweidong
 ** Last modified data: 2015-6-10
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "encode.h"

static int ascii_is_supported(unsigned char *buf)
{
    return 1;
}

static int ascii_get_char_code(unsigned char *buf, unsigned char *code)
{
    return 0;
}

struct encode_ops ascii_encode_ops = {
        .name = "ascii",
        .is_supported = ascii_is_supported,
        .get_char_code = ascii_get_char_code,
};

int ascii_encode_init(void)
{
    if (register_encode_ops(&ascii_encode_ops) == -1) {
        ERR_PRINT("fail to register %s encode ops\n", ascii_encode_ops.name);
        return -1;
    }
    return 0;
}
