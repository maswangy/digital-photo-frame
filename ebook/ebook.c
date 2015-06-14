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

void lcd_draw_font_8x8(int x, int y, unsigned char *bitmap)
{
    int i;
    int j;

    PRINT_DBG("bitmap:\n");
    for (i = 0; i < 8; i++) {
        PRINT_DBG("%x\n",bitmap[i]);
#if 0
        for (j = 7; j >= 0; j--) {
            if (bitmap[i] & (1<<j))
                lcd_draw_pixel(x+7-j, y+i, 0x0000ff);           // blue
            else
                lcd_draw_pixel(x+7-j, y+i, 0x0);
        }
#endif
    }
}

int show_one_page(struct txt_info *txt)
{
    struct encode_ops *ecd_ops;
    struct bitmap_ops *bmp_ops;
    int len;
    unsigned int code;
    unsigned char *bitmap = NULL;
    const unsigned char *txt_buf;

    if (txt == NULL) {
        return -1;
    }
    ecd_ops = txt->ecd_ops;
    bmp_ops = txt->bmp_ops;
    len = txt->length;
    txt_buf = txt->buf;

    while (len--) {
        if (ecd_ops->get_char_code(txt_buf++, &code) == -1) {
            return -1;
        }

        if (bmp_ops->get_char_bitmap(code, &bitmap) == -1) {
            return -1;
        }
        lcd_draw_font_8x8(1920/2, 1080/2, bitmap);
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

    if (bitmap_init() == -1) {
        PRINT_ERR("fail to init bitmap module\n");
        goto exit;
    }

    bitmap_list();

    if (bitmap_select(&txt) == -1) {
        PRINT_ERR("fail to select bitmap type\n");
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
