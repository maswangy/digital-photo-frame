/******************************************************************************** 
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : input.h
** Author            : wuweidong  
** Description       :
** Todo              :
********************************************************************************/

#ifndef __INPUT_H_
#define __INPUT_H_

#include "config.h"

#define INPUT_TYPE_UNKNOWN      (-1)
#define INPUT_TYPE_STDIN        (1)
#define INPUT_TYPE_TOUCHSCREEN  (2)

#define INPUT_VALUE_UNKNOWN     (-1)
#define INPUT_VALUE_UP          (1)
#define INPUT_VALUE_DOWN        (2)
#define INPUT_VALUE_EXIT        (3)

struct input_event {
    struct timeval time;
    int type;
    int value;
};

struct input_ops {
    struct list_head list;
    char *name;
    int type;
    int (*init)(void);
    void (*exit)(void);
    int (*get_input_event)(struct input_event *event);
};

int register_input_ops(struct input_ops *ops);
int deregister_input_ops(struct input_ops *ops);
int input_init(void);
int input_exit(void);
int get_input_ops_event(struct input_event *event);
int input_ops_init(void);

int stdin_input_init(void);
int stdin_input_exit(void);
#endif /* __INPUT_H_ */
