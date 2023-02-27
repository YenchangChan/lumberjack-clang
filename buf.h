#ifndef __LJ_BUF_H__
#define __LJ_BUF_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lumberjack_buf_t {
    char    *data;
    int     cap;
    int     size;
}lumberjack_buf_t;

#define LJ_BUF_CAP_DEFAULT 1046576      // 1M

lumberjack_buf_t *lumberjack_buf_create();
lumberjack_buf_t *lumberjack_buf_new_size(int cap);
void lumberjack_buf_append(lumberjack_buf_t *buf, char *data, int size);
void lumberjack_buf_scale_up(lumberjack_buf_t *buf, int cap);
void lumberjack_buf_scale_down(lumberjack_buf_t *buf, int cap);
void lumberjack_buf_reset(lumberjack_buf_t *buf);
void lumberjack_buf_destory(lumberjack_buf_t *buf);


#ifdef __cplusplus
}
#endif

#endif