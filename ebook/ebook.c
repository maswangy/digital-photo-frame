/********************************************************************************
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : ebook.c
** Author            : wuweidong
** Last modified data: 2015-6-10
** Description       :
** Todo              :
********************************************************************************/

#include "config.h"

struct txt_info {
    int fd;
    char *map_buf;
    struct stat stat_buf;
};
struct txt_info txt;

int open_txt(char *name)
{
    struct stat *stat_buf;
    char *map_buf;
    int i = 0;

    if ((txt.fd = open(name, O_RDONLY)) == -1) {
        ERR_PRINT("fail to open %s\n", name);
        return -1;
    }

    if (fstat(txt.fd, &(txt.stat_buf)) == -1) {
        ERR_PRINT("fail to fstat %d\n", txt.fd);
        return -1;
    }

    stat_buf = &(txt.stat_buf);
    txt.map_buf = (char *)mmap(NULL, stat_buf->st_size, PROT_READ, MAP_SHARED, txt.fd, 0);
    if (txt.map_buf == MAP_FAILED) {
        ERR_PRINT("fail to mmap %d\n", txt.fd);
        return -1;
    }

    DBG_PRINT("txt:\n");
    map_buf = txt.map_buf;
    for (i=0; i<32; i++) {
        DBG_PRINT("%02x ", (0xff & map_buf[i]));
        if (!((i+1)%16) ) {
            DBG_PRINT("\n");
        }
    }
    return 0;
}

void close_txt()
{
    struct stat *buf = &(txt.stat_buf);
    char *map_buf = txt.map_buf;
    munmap((void*)map_buf, buf->st_size);
    close(txt.fd);
}

void print_usage(int argc, char **argv)
{
    ERR_PRINT("Usage:%s filename\n", argv[0]);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        print_usage(argc, argv);
        return -1;
    }
    if (open_txt(argv[1]) == -1) {
        ERR_PRINT("fail to open txt txt %s\n", argv[1]);
    }
#if 0
    if (encode_init() == -1) {
        ERR_PRINT("fail to init encode module\n");
    }
#endif

    close_txt();

    return 0;
}
