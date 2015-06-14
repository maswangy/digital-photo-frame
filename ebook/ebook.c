/********************************************************************************
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ebook.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "encode.h"
#include "bitmap.h"
#include "config.h"

struct txt_info txt;

int show_one_page(struct txt_info *txt)
{
    struct encode_ops *ecd_ops;
    int len;
    unsigned int code;
    const unsigned char *buf;
    if (txt == NULL) {
        return -1;
    }
    ecd_ops = txt->ecd_ops;
    len = txt->length;
    buf = txt->buf;

    while (len--) {
        if (ecd_ops->get_char_code(buf++, &code) == -1) {
            return -1;
        }
        PRINT_DBG("%02x ", code);
        if (!(len % 16)) {
            PRINT_DBG("\n");
        }
    }

    return 0;
}

int open_txt(char *name)
{
    struct stat stat_buf;
    const unsigned char *buf;
    int i = 0;

    if ((txt.fd = open(name, O_RDONLY)) == -1) {
        PRINT_ERR("fail to open %s\n", name);
        return -1;
    }

    if (fstat(txt.fd, &stat_buf) == -1) {
        PRINT_ERR("fail to fstat %d\n", txt.fd);
        return -1;
    }

    txt.buf = (const unsigned char *)mmap(NULL, stat_buf.st_size, PROT_READ, MAP_SHARED, txt.fd, 0);
    if (txt.buf == MAP_FAILED) {
        PRINT_ERR("fail to mmap %d\n", txt.fd);
        return -1;
    }
    txt.length = stat_buf.st_size;

    PRINT_DBG("txt:\n");
    buf = txt.buf;
    for (i=0; i<32; i++) {
        PRINT_DBG("%02x ", (0xff & buf[i]));
        if (!((i+1)%16) ) {
            PRINT_DBG("\n");
        }
    }
    return 0;
}

void close_txt()
{
    munmap((void*)txt.buf, txt.length);
    close(txt.fd);
}

void print_usage(int argc, char **argv)
{
    PRINT_ERR("Usage:%s filename\n", argv[0]);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        print_usage(argc, argv);
        return -1;
    }
    if (open_txt(argv[1]) == -1) {
        PRINT_ERR("fail to open txt file %s\n", argv[1]);
        return -1;
    }

    if (encode_init() == -1) {
        PRINT_ERR("fail to init encode module\n");
        goto exit;
    }
    encode_list();
    if (encode_select(&txt) == -1) {
        PRINT_ERR("fail to select encode type\n");
        goto exit;
    }

    show_one_page(&txt);

    if (encode_exit() == -1) {
        PRINT_ERR("fail to exit encode module\n");
        goto exit;
    }

exit:
    close_txt();
    return 0;
}
