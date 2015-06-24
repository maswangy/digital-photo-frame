/********************************************************************************
 ** Copyright (C) 2015 <ericrock@foxmail.com>
 ** File name         : ebook.c
 ** Author            : wuweidong
 ** Description       :
 ** Todo              :
 ********************************************************************************/

#include <stdlib.h>
#include "encode.h"
#include "bitmap.h"
#include "display.h"
#include "config.h"

struct txt_info txt;
static struct page page_entry;
static struct page *cur_page;

int show_one_page(const unsigned char *page_buf)
{
    int startx, starty;
    struct encode_ops *ecd_ops;
    struct bitmap_ops *bmp_ops;
    struct display_ops *dsp_ops;
    struct char_frame cf;
    unsigned int code;
    unsigned char *bitmap = NULL;
    const unsigned char *cur_buf = page_buf;
    int len;

    ecd_ops = txt.ecd_ops;
    bmp_ops = txt.bmp_ops;
    dsp_ops = txt.dsp_ops;

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
    while (cur_buf < (page_buf + txt.length)) {
        if ((len = ecd_ops->get_char_code(cur_buf, &code)) == -1) {
            return (cur_buf - page_buf);
        }                
        if (bmp_ops->get_char_bitmap(code, &bitmap, &cf) == -1) {
            return (cur_buf - page_buf);
        }
        // if need change page
        if ((cf.ymin + cf.height) > dsp_ops->yres ) {
            return (cur_buf - page_buf);
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

    if (next_page->buf == NULL) {
        PRINT_DBG("page%d\n", cur_page->id);
        len = show_one_page(cur_page->buf);
    } else {
        PRINT_DBG("page%d\n", cur_page->id);
        len = show_one_page(next_page->buf);
        cur_page = next_page;
    }
    PRINT_DBG("len=%d\n", len);
    // still have next page ?
    if ((cur_page->buf + len - txt.buf) >= txt.length) {
        PRINT_DBG("end of novel\n");
        return 1;
    }
    struct page *new_page = malloc(sizeof(struct page));
    new_page->buf = cur_page->buf + len;
    new_page->id = cur_page->id + 1;
    PRINT_DBG("add new page,id=%d\n\n", new_page->id);
    list_add_tail(&(new_page->list), &(page_entry.list));

    page_list();
    return 0;
}

int show_prev_page(void)
{
    struct list_head *cur_list = &(cur_page->list);
    struct page *prev_page = list_entry(cur_list->prev, struct page, list);
    if (prev_page->buf == NULL) {
        PRINT_DBG("page%d\n", cur_page->id);
        show_one_page(cur_page->buf);
    } else {
        PRINT_DBG("page%d\n", cur_page->id);
        show_one_page(prev_page->buf);
        cur_page = prev_page;
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

void print_usage(int argc, char **argv)
{
    PRINT_ERR("Usage:%s filename\n", argv[0]);
}

int main(int argc, char **argv)
{
    char c;

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

    while (1) { 
        PRINT_INFO("n:next page, p:previous page\n");
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
