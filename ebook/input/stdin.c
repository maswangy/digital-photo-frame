/******************************************************************************** 
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : stdin.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include <termios.h>
#include <unistd.h>
#include <time.h>
#include "input.h"

#define NB_ENABLE (1)
#define NB_DISABLE (0)

static struct termios orig_ttystate;

static void tty_nonblock(int state)
{
    struct termios ttystate;

    tcgetattr(STDIN_FILENO, &ttystate);                     // get the terminal state
    orig_ttystate.c_lflag = ttystate.c_lflag;
    orig_ttystate.c_cc[VMIN] = ttystate.c_cc[VMIN];

    if (state == NB_ENABLE) {
        ttystate.c_lflag &= ~ICANON;                        // turn off canonical mode
        ttystate.c_cc[VMIN] = 1;                            // minimum of number input read
    }
    else if (state == NB_DISABLE) {
        ttystate.c_lflag |= ICANON;                         // turn on canonical mode
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);            // set the terminal attributes.
}

static int stdin_get_input_event(struct input_event *event)
{
    struct timeval tv;
    char c;
    fd_set fds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);                             // STDIN_FILENO is 0
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (FD_ISSET(STDIN_FILENO, &fds)) {
        c = getchar();
        // PRINT_DBG("c=%c\n",c);
        event->type = INPUT_TYPE_STDIN;
        switch(c) {
        case '+':
            event->value = INPUT_VALUE_DOWN;
            break;
        case '-':
            event->value = INPUT_VALUE_UP;
            break;
        case '\n':
            event->value = INPUT_VALUE_EXIT;
            break;
        default:
            event->value = INPUT_VALUE_UNKNOWN;
            break;
        }
        return 0;
    } else {
        return -1;
    }
}

static int stdin_init(void)
{
    tty_nonblock(NB_ENABLE);
    return 0;
}

static void stdin_exit(void)
{
    struct termios ttystate;

    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag = orig_ttystate.c_lflag;
    ttystate.c_cc[VMIN] = orig_ttystate.c_cc[VMIN];
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

static struct input_ops stdin_input_ops = {
        .name = "stdin",
        .type = INPUT_TYPE_STDIN,
        .init = stdin_init,
        .exit = stdin_exit,
        .get_input_event  = stdin_get_input_event,
};

int stdin_input_init(void)
{
    if (register_input_ops(&stdin_input_ops) == -1) {
        PRINT_ERR("fail to register %s input ops\n", stdin_input_ops.name);
        return -1;
    }
    return 0;
}

int stdin_input_exit(void)
{
    if (deregister_input_ops(&stdin_input_ops) == -1) {
        PRINT_ERR("fail to deregister %s input ops\n", stdin_input_ops.name);
        return -1;
    }
    return 0;
}
