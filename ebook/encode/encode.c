/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : encode.c
** Author            : wuweidong  
** Last modified data: 2015-6-11
** Description       :
** Todo              :
********************************************************************************/

#include "config.h"
#include "encode.h"

int register_encode_ops(struct encode_ops *ops)
{
    return 0;
}

int encode_init(void)
{
    if (ascii_encode_init() == -1) {
        ERR_PRINT("fail to init encode\n");
        return -1;
    }
    return 0;
}
