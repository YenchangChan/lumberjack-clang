#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "buf.h"
#include "constant.h"

lumberjack_buf_t *lumberjack_buf_new_size(int cap){
    lumberjack_buf_t *buf = calloc(1, sizeof(lumberjack_buf_t));
    buf->size = 0;
    buf->cap = cap;
    buf->data = (char *)calloc(1, cap);

    return buf;
}

lumberjack_buf_t *lumberjack_buf_create(){
    return lumberjack_buf_new_size(LJ_BUF_CAP_DEFAULT);
}

void lumberjack_buf_append(lumberjack_buf_t *buf, char *data, int size){
    if  (buf->size + size > buf->cap) {
        lumberjack_buf_scale_up(buf, buf->size + size);
    }

    memcpy(buf->data + buf->size, data, size);
    buf->size += size;
}

void lumberjack_buf_scale_up(lumberjack_buf_t *buf, int cap){
    int new_cap = cap << 1;
    buf->data = realloc(buf->data, new_cap);
    buf->cap = new_cap;
}

void lumberjack_buf_scale_down(lumberjack_buf_t *buf, int cap){
    int new_cap = cap >> 1;
    buf->data = realloc(buf->data, new_cap);
    buf->cap = new_cap;
}

void lumberjack_buf_reset(lumberjack_buf_t *buf){
    static int reset_times = 0;
    if ((buf->size << 1) < buf->cap) {
        reset_times++;
    }
    if (reset_times > 3) {
        lumberjack_buf_scale_down(buf, buf->cap);
        reset_times = 0;
    }
    memset(buf->data, 0, buf->size);
    buf->size = 0;
}

void lumberjack_buf_destory(lumberjack_buf_t *buf){
    _FREE(buf->data);
    _FREE(buf);
}

