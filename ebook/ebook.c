/********************************************************************************
** Copyright (C) 2015 <ericrock@foxmail.com>
** File name         : ebook.c
** Author            : wuweidong
** Last modified data: 2015-6-10
** Description       :
** Todo              :
********************************************************************************/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "config.h"

struct file_info {
    int fd;
    char *map_buf;
    struct stat stat_buf;
};
struct file_info file;

int open_file(char *name)
{
    struct stat *stat_buf;
    char *map_buf;
    int i = 0;

    if ((file.fd = open(name, O_RDONLY)) == -1) {
        ERR_PRINT("fail to open %s\n", name);
        return -1;
    }

    if (fstat(file.fd, &(file.stat_buf)) == -1) {
        ERR_PRINT("fail to fstat %d\n", file.fd);
        return -1;
    }

    stat_buf = &(file.stat_buf);
    file.map_buf = (char *)mmap(NULL, stat_buf->st_size, PROT_READ, MAP_SHARED, file.fd, 0);
    if (file.map_buf == MAP_FAILED) {
        ERR_PRINT("fail to mmap %d\n", file.fd);
        return -1;
    }

    DBG_PRINT("file:\n");
    map_buf = file.map_buf;
    for (i=0; i<32; i++) {
        DBG_PRINT("%02x ", map_buf[i]);
        if (!((i+1)%16) ) {
            DBG_PRINT("\n");
        }
    }
    return 0;
}

void close_file()
{
    struct stat *buf = &(file.stat_buf);
    char *map_buf = file.map_buf;
    munmap((void*)map_buf, buf->st_size);
    close(file.fd);
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
    if (open_file(argv[1]) == -1) {
        ERR_PRINT("fail to open txt file %s\n", argv[1]);
    }
#if 0
    if (encode_init() == -1) {
        ERR_PRINT("fail to init encode module\n");
    }
#endif

    close_file();

    return 0;
}
