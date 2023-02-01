#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "metric.h"
#include "../constant.h"

lumberjack_metrics_t *metrics_init(int interval){
	lumberjack_metrics_t *metrics = calloc(1, sizeof(lumberjack_metrics_t));
	metrics->interval = interval;
	return metrics;
}
void metrics_add_lines(lumberjack_metrics_t *metrics, int lines){
	metrics->ack_lines += lines;
}
void metrics_add_bytes(lumberjack_metrics_t *metrics, int bytes){
	metrics->send_bytes += bytes;
}
void metrics_print(lumberjack_metrics_t *metrics){
	time_t now = time(NULL);
	if (metrics->time == 0)
	{
		metrics->time = now;
	}
	else if (now - metrics->time >= metrics->interval)
	{
		printf("[metrics-logging] send_lines: %d, send_bytes:%d, tps: %g/s, %g MB/s\n",
			   metrics->ack_lines, metrics->send_bytes,
			   (metrics->ack_lines - metrics->prev_lines) / (metrics->interval * 1.0),
			   (metrics->send_bytes - metrics->prev_bytes) / (metrics->interval * 1.0 * 1024 * 1024));
		metrics->time = now;
		metrics->prev_lines = metrics->ack_lines;
		metrics->prev_bytes = metrics->send_bytes;
	}
}
void metrics_destroy(lumberjack_metrics_t *metrics){
	_FREE(metrics);
}