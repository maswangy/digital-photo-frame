/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : input.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/
#include <sys/select.h>
#include "input.h"

static struct list_head entry;
static fd_set input_rfds;
static int max_fd;
int register_input_ops(struct input_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_add_tail(&(ops->list), &entry);
    return 0;
}

int deregister_input_ops(struct input_ops *ops)
{
    if (ops == NULL)
        return -1;
    list_del(&(ops->list));
    return 0;
}

int input_get_event(struct input_event *event)
{
    int ret = select(max_fd, &input_rfds, NULL, NULL, NULL);
    if (ret != -1 && ret) {
        struct list_head *list;
        list_for_each(list, &entry) {
            struct input_ops *ops = list_entry(list, struct input_ops, list);
            if (FD_ISSET(ops->fd, &input_rfds)) {
                if (ops->get_input_event && ops->get_input_event(event) == 0) {
                    return 0;
                }
            }
        }
    }
    return -1;
}

int input_ops_init(void)
{
    struct list_head *list;

    list_for_each(list, &entry) {
        struct input_ops *ops = list_entry(list, struct input_ops, list);
        if (ops->init && ops->init() == -1) {
            PRINT_ERR("fail to init input :%s\n", ops->name);
            return -1;
        }
        FD_SET(ops->fd, &input_rfds);
        if (max_fd <= ops->fd) {
            max_fd = ops->fd + 1;
        }
    }
    return 0;
}

int input_init(struct txt_info *txt)
{
    INIT_LIST_HEAD(&entry);
    FD_ZERO(&input_rfds);
    if (stdin_input_init() == -1) {
        PRINT_ERR("fail to init stdin input\n");
        return -1;
    }

    return 0;
}

int input_exit(void)
{
    if (stdin_input_exit() == -1) {
        PRINT_ERR("fail to exit stdin input\n");
        return -1;
    }
    return 0;
}

