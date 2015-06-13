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
#include "list.h"

struct encode_ops {
    struct list_head list;
    char *name;
    int type;
    int (*is_supported)(const unsigned char *buf, int length);
    int (*get_char_code)(const unsigned char *buf, unsigned char *code);
};

int register_encode_ops(struct encode_ops *ops);
int deregister_encode_ops(struct encode_ops *ops);
int encode_init(void);
int encode_exit(void);
int encode_select(struct txt_info *txt);
void encode_list(void);

int ascii_encode_init(void);
int ascii_encode_exit(void);

#endif /* __ENCODE_H_ */
