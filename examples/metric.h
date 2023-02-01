#ifndef METRIC_H_
#define METRIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

typedef struct lumberjack_metrics_t {
    time_t time;
    int prev_lines;
    int ack_lines;
    int prev_bytes;
    int send_bytes;
    int interval;
} lumberjack_metrics_t;

lumberjack_metrics_t *metrics_init(int interval);
void metrics_add_lines(lumberjack_metrics_t *metrics, int lines);
void metrics_add_bytes(lumberjack_metrics_t *metrics, int bytes);
void metrics_print(lumberjack_metrics_t *metrics);
void metrics_destroy(lumberjack_metrics_t *metrics);

#ifdef __cplusplus
}
#endif

#endif