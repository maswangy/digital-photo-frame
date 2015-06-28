/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : encode.h
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#ifndef __ENCODE_H_
#define __ENCODE_H_

#include "config.h"

struct encode_ops {
    struct list_head list;
    char *name;
    int type;
    int (*is_supported)(const unsigned char *buf, int length);
    int (*get_char_code)(const unsigned char *buf, unsigned int *code);
};

int register_encode_ops(struct encode_ops *ops);
int deregister_encode_ops(struct encode_ops *ops);
int encode_init(void);
int encode_exit(void);
int encode_select(struct txt_info *txt);
void encode_list(void);

int ascii_encode_init(void);
int ascii_encode_exit(void);
int iso8859_encode_init(void);
int iso8859_encode_exit(void);
int utf8_encode_init(void);
int utf8_encode_exit(void);

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

extern char text_chars[256];
#endif /* __ENCODE_H_ */
