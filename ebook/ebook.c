/********************************************************************************
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ebook.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include "encode.h"
#include "bitmap.h"
#include "display.h"
#include "config.h"

struct txt_info txt;

int show_one_page(struct txt_info *txt)
{
    int startx, starty;
    struct encode_ops *ecd_ops;
    struct bitmap_ops *bmp_ops;
    struct display_ops *dsp_ops;
    struct char_frame cf;
    unsigned int code;
    unsigned char *bitmap = NULL;
    const unsigned char *cur_buf;
    int len;

    if (txt == NULL) {
        return -1;
    }
    ecd_ops = txt->ecd_ops;
    bmp_ops = txt->bmp_ops;
    dsp_ops = txt->dsp_ops;
    cur_buf = txt->buf;

#if defined( __x86_64__) || defined(__i386__)
    startx = dsp_ops->xres / 2;
    starty = 0;
#else               // ARM
    startx = starty = 0;
#endif
    PRINT_DBG("start(startx=%d, starty=%d) xres=%d yres=%d\n", startx, starty, dsp_ops->xres, dsp_ops->yres);
    dsp_ops->clear_screen(0xE7DBB5);
    cf.xmin = startx;
    cf.ymin = starty;
    while (cur_buf < (txt->buf + txt->length)) {
        if ((len = ecd_ops->get_char_code(cur_buf, &code)) == -1) {
            return (txt->buf - cur_buf);
        }                
        if (bmp_ops->get_char_bitmap(code, &bitmap, &cf) == -1) {
            return (txt->buf - cur_buf);
        }
        // if need change page
        if ((cf.ymin + cf.height) > dsp_ops->yres ) {
            return (txt->buf - cur_buf);
        }

#if 0       
        PRINT_DBG("code:%x\n", code);
        PRINT_DBG("xmin:%d\n", cf.xmin);
        PRINT_DBG("ymin:%d\n", cf.ymin);
        PRINT_DBG("xmax:%d\n", cf.xmax);
        PRINT_DBG("ymax:%d\n", cf.ymax);
        PRINT_DBG("width:%d\n", cf.width);
        PRINT_DBG("height:%d\n\n", cf.height);
#endif
        cur_buf += len;

        // handle enter
        // 1. windows enter: 0d 0a = \r \n
        // 2. unix enter: 0a = \n
        if ( code == 0x0d0a || code == '\n') {
            cf.xmin = startx;
            cf.ymin = cf.ymin + cf.height;
            continue;
        }
        // handle tab
        if ( (unsigned char)code == '\t') {
            cf.xmin += 3*8;
            continue;
        }
        
        // if need change line
        // 1. meet enter;
        // 2. meet x edge;
        if ((cf.xmin + cf.width) > dsp_ops->xres) {
            cf.xmin = startx;
            cf.ymin += cf.height;
        }
                
        int i, j, k;
        unsigned char byte;
        for (i = 0; i < cf.height; i++) {          
            for (k = 0; k < cf.width/8; k++) {       
                byte = bitmap[i*cf.width/8 + k];
                for (j = 7; j >= 0; j--) {
                    if (byte & (1<<j))
                        dsp_ops->draw_pixel(cf.xmin + (8*k) + (7-j), cf.ymin + i, 0x0);     
                }
            }
        }
        cf.xmin += cf.width;
        // sleep(1);
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

    if (display_init() == -1) {
        PRINT_ERR("fail to init display module\n");
        goto exit;
    }

    display_list();

    if (display_select(&txt) == -1) {
        PRINT_ERR("fail to select display type\n");
        goto exit;
    }

    show_one_page(&txt);

    if (display_exit() == -1) {
        PRINT_ERR("fail to exit encode module\n");
        goto exit;
    }

    if (bitmap_exit() == -1) {
        PRINT_ERR("fail to exit encode module\n");
        goto exit;
    }

    if (encode_exit() == -1) {
        PRINT_ERR("fail to exit encode module\n");
        goto exit;
    }

    exit:
    close_txt();
    return 0;
}
