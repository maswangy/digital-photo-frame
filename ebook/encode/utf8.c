/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : utf8.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "config.h"
#include "encode.h"

static int utf8_is_supported(const unsigned char *buf, int length)
{
    if (buf == NULL || length < 0) {
        return 0;
    }

    // work for file with BOM(Byte Order Mark)
    if (length>=3 && buf[0]==0xef && buf[1]==0xbb && buf[2]==0xbf)
        return 1;

    // in case file without BOM
    int i = 0;
    int n = 0;
    for (i = 0; i < length; i++) {
        if ((buf[i] & 0x80) == 0) {     /* 0xxxxxxx is plain ASCII */
            if (text_chars[buf[i]] != T) {
                break;
            }
        } else if ((buf[i] & 0x40) == 0) {   /* 10xxxxxx never happen */
            break;
        } else {
            int following;
            if ((buf[i] & 0x20) == 0) {         /* 110xxxxx */
                following = 1;
            } else if ((buf[i] & 0x10) == 0) {  /* 1110xxxx */
                following = 2;
            } else if ((buf[i] & 0x08) == 0) {  /* 11110xxx */
                following = 3;
            } else if ((buf[i] & 0x04) == 0) {  /* 111110xx */
                following = 4;
            } else if ((buf[i] & 0x02) == 0) {  /* 1111110x */
                following = 5;
            } else {
                break;
            }
            for (n = 0; n < following; n++) {
                i++;
                if (i > length)
                    break;
                // left byte must begin with 10
                if ((buf[i] & 0x80) == 0 || (buf[i] & 0x40))
                    break;
            }
        }
    }
    if ( i != length ) {
        // PRINT_DBG("Not utf8 code:%x\n", buf[i]);
        return 0;
    } else {
        return 1;
    }
}

static int utf8_get_char_code(const unsigned char *buf, unsigned int *code)
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

static struct encode_ops utf8_encode_ops = {
        .name = "utf8",
        .type = ENCODE_UTF8,
        .is_supported = utf8_is_supported,
        .get_char_code = utf8_get_char_code,
};

int utf8_encode_init(void)
{
    if (register_encode_ops(&utf8_encode_ops) == -1) {
        PRINT_ERR("fail to register %s encode ops\n", utf8_encode_ops.name);
        return -1;
    }
    return 0;
}

int utf8_encode_exit(void)
{
    if (deregister_encode_ops(&utf8_encode_ops) == -1) {
        PRINT_ERR("fail to deregister %s encode ops\n", utf8_encode_ops.name);
        return -1;
    }
    return 0;
}
