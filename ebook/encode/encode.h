/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : encode.h
** Author            : wuweidong  
** Last modified data: 2015-6-11
** Description       :
** Todo              :
********************************************************************************/

#ifndef __ENCODE_H_
#define __ENCODE_H_

struct encode_ops {
    char *name;
    int (*is_supported)(unsigned char *buf);
    int (*get_char_code)(unsigned char *buf, unsigned char *code);
};

int register_encode_ops(struct encode_ops *ops);
int encode_init(void);

int ascii_encode_init(void);

#endif /* __ENCODE_H_ */
