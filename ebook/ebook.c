/********************************************************************************
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ebook.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "encode.h"
#include "bitmap.h"
#include "display.h"
#include "config.h"

struct txt_info txt;
static struct page page_entry;
static struct page *cur_page;

int show_one_page(struct page *p)
{
    int startx, starty;
    struct encode_ops *ecd_ops;
    struct bitmap_ops *bmp_ops;
    struct display_ops *dsp_ops;
    struct char_frame cf;
    unsigned int code;
    unsigned char *bitmap = NULL;
    unsigned char *cur_buf = p->buf;
    int len;

    PRINT_DBG("show page: %d\n", p->id);

    ecd_ops = txt.ecd_ops;
    bmp_ops = txt.bmp_ops;
    dsp_ops = txt.dsp_ops;

#if defined( __x86_64__) || defined(__i386__)
    startx = dsp_ops->xres / 2;
    starty = dsp_ops->yres / 2;
#else               // ARM
    startx = starty = 0;
#endif
    PRINT_DBG("start(startx=%d, starty=%d) xres=%d yres=%d txt.length=%d\n", startx, starty, dsp_ops->xres, dsp_ops->yres, txt.length);
    dsp_ops->clear_screen(0xE7DBB5);
    cf.xmin = startx;
    cf.ymin = starty;
    cf.disp_xres = dsp_ops->xres;
    cf.disp_yres = dsp_ops->yres;
    while (cur_buf < (p->buf + txt.length)) {
        if ((len = ecd_ops->get_char_code(cur_buf, &code)) == -1) {
            PRINT_DBG("Fail to get_char_code\n");
            return (cur_buf - p->buf);
        }                
        if (bmp_ops->get_char_bitmap(code, &bitmap, &cf) == -1) {
            PRINT_DBG("Fail to get_char_bitmap\n");
            return (cur_buf - p->buf);
        }
#if 1
        // if need change page
        if ((cf.ymin + cf.height) > dsp_ops->yres ) {
            return (cur_buf - p->buf);
        }
#endif

#if 1
        PRINT_DBG("code:%x len=%d\n", code, len);
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
            cf.xmin += 3 * 8;
            continue;
        }

        // handle space
        if ( (unsigned char)code == ' ') {
            cf.xmin += 1 * 8;
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
                    if (byte & (1<<j)) {
                        //PRINT_DBG("draw_pixel\n");
                        dsp_ops->draw_pixel(cf.xmin + (8*k) + (7-j), cf.ymin + i, 0x0);
                    }
                }
            }
        }

        cf.xmin += cf.width;
        // sleep(1);
    }
    PRINT_DBG("show_one_page end\n");
    return (cur_buf - p->buf);
}

void page_list(void)
{
    struct list_head *list;

    PRINT_DBG("page_list:\n");
    list_for_each(list, &(page_entry.list)) {
        struct page *p = list_entry(list, struct page, list);
        PRINT_INFO("%d\n", p->id);
    }
}

int show_next_page(void) 
{   
    int len = 0;
    struct list_head *cur_list = &(cur_page->list);
    struct page *next_page = list_entry(cur_list->next, struct page, list);

    page_list();

    if (next_page->buf == NULL) {
        len = show_one_page(cur_page);
    } else {
        len = show_one_page(next_page);
        cur_page = next_page;
        cur_list = &(cur_page->list);
        next_page = list_entry(cur_list->next, struct page, list);
    }
    PRINT_DBG("len=%d\n", len);

    // end of novel
    if ((cur_page->buf + len - txt.buf) >= txt.length) {
        PRINT_DBG("end of novel\n");
        return 1;
    }

    if (next_page->buf == (cur_page->buf + len) && next_page->id == (cur_page->id + 1)) {
        return 0;
    } else {
        struct page *new_page = malloc(sizeof(struct page));
        new_page->buf = cur_page->buf + len;
        new_page->id = cur_page->id + 1;
        PRINT_DBG("add new page: %d\n\n", new_page->id);
        list_add_tail(&(new_page->list), &(page_entry.list));
    }
    return 0;
}

int show_prev_page(void)
{
    struct list_head *cur_list = &(cur_page->list);
    struct page *prev_page = list_entry(cur_list->prev, struct page, list);
    if (prev_page->buf == NULL) {
        show_one_page(cur_page);
    } else {
        show_one_page(prev_page);
        cur_page = prev_page;
    }
    return 0;
}

int open_txt(char *txt_file, char *ttc_file)
{
    struct stat stat_buf;
    const unsigned char *buf;
    int i = 0;

    strcpy(txt.ttc_path, ttc_file);
    if ((txt.fd = open(txt_file, O_RDONLY)) == -1) {
        PRINT_ERR("fail to open %s\n", txt_file);
        return -1;
    }

    if (fstat(txt.fd, &stat_buf) == -1) {
        PRINT_ERR("fail to fstat %d\n", txt.fd);
        return -1;
    }

    txt.buf = (unsigned char *)mmap(NULL, stat_buf.st_size, PROT_READ, MAP_SHARED, txt.fd, 0);
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

void txt_head_remove(void)
{
    unsigned char *head;
    if (txt.ecd_ops->type == ENCODE_UTF8) {
        head = txt.buf;
        if (head[0] == 0xef && head[1] == 0xbb && head[2] == 0xbf) {
            txt.buf = txt.buf + 3;
            txt.length -= 3;
        }
    }

    if (txt.ecd_ops->type == ENCODE_UTF16BE) {
        head = txt.buf;
        if (head[0]==0xfe && head[1]==0xff) {
            txt.buf = txt.buf + 2;
            txt.length -= 2;
        }
    }

    if (txt.ecd_ops->type == ENCODE_UTF16LE) {
        head = txt.buf;
        if (head[0]==0xff && head[1]==0xfe) {
            txt.buf = txt.buf + 2;
            txt.length -= 2;
        }
    }
}

void print_usage(int argc, char **argv)
{
    PRINT_ERR("Usage:%s txt_file font_file\n", argv[0]);
}

int main(int argc, char **argv)
{
    char c;

    if (argc != 3) {
        print_usage(argc, argv);
        return -1;
    }
    if (open_txt(argv[1], argv[2]) == -1) {
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

    txt_head_remove();

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

    // head page, not have any data
    page_entry.buf = NULL;
    page_entry.id = -1;
    INIT_LIST_HEAD(&(page_entry.list));

    // first page
    cur_page = malloc(sizeof(struct page));
    cur_page->id = 1;
    cur_page->buf = txt.buf;
    list_add(&(cur_page->list), &(page_entry.list));
    show_next_page();

    PRINT_INFO("\n\nusage: n[next page], p[previous page]\n");
    while (1) { 
        switch (c = getchar()) {
        case 'n':
            show_next_page();
            break;
        case 'p':
            show_prev_page();
            break;
        default:
            break;
        }
    }
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
