/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : input.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/
#include "input.h"

static struct list_head entry;
static struct input_event g_event;
static pthread_mutex_t mutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;

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
    pthread_mutex_lock(&mutex);
    // if cond is not true, mutex will be unlock & thread will sleep;
    // if cond is true, mutex will lock & thread will be wakeup;
    pthread_cond_wait(&cond, &mutex);
    *event = g_event;
    pthread_mutex_unlock(&mutex);
    return 0;
}

void *input_get_event_thread(void *arg)
{

    struct input_event event;
    int (*get_input_event)(struct input_event *event) = (int (*)(struct input_event *))arg;
    while (1) {
        if (0 == get_input_event(&event)) { // if no input, get_input_event will sleep.
            pthread_mutex_lock(&mutex);
            g_event = event;
            // wake up main thread
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
    }
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
        // create thread
        if (pthread_create(&ops->pid, NULL, input_get_event_thread, ops->get_input_event) !=0 ) {
            PRINT_ERR("fail to cread %s get_event thread\n", ops->name);
            return -1;
        }
    }
    return 0;
}

int input_init(struct txt_info *txt)
{
    INIT_LIST_HEAD(&entry);
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

