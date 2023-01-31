#include "../lumberjack.h"

typedef struct lumberjack_metrics_t {
    time_t time;
    int prev_lines;
    int ack_lines;
    int prev_bytes;
    int send_bytes;
    int interval;
} lumberjack_metrics_t;

int main(void){
    lumberjack_config_t config  = {
        .hosts = "192.168.110.8",
        .port  = 7070,
        .compress_level =  3,
        .client_port_range = "60000,60100",
        .bandwidth = 0,
        .protocol = LUMBERJACK_PROTO_VERSION_V2,
        .timeout = 10,
        .batch = 200,
        .with_ssl = false,
    };

    lumberjack_metrics_t metrics = {
        .interval = 5,
        .time = 0,
        .prev_lines = 0,
        .ack_lines = 0,
        .prev_bytes = 0,
        .send_bytes = 0,
    };
    // lumberjack_config_t config = {0};
    lumberjack_client_t *client = lumberjack_new_client(&config);
    printf("client created %s:%d \n", client->conn.host, client->conn.port);
    client->start(client);
    printf("client started\n");
    while (1) {
        time_t now = time(NULL);
        if (metrics.time == 0) {
            metrics.time = now;
        } else if (now - metrics.time >= metrics.interval) {
            printf("[metrics-logging] send_lines: %d, send_bytes:%d, tps: %g/s, %g MB/s\n",
                   metrics.ack_lines, metrics.send_bytes,
                   (metrics.ack_lines - metrics.prev_lines) / (metrics.interval * 1.0),
                   (metrics.send_bytes - metrics.prev_bytes) / (metrics.interval * 1.0 * 1024 * 1024));
            metrics.time = now;
            metrics.prev_lines = metrics.ack_lines;
            metrics.prev_bytes = metrics.send_bytes;
        }
        if (client->is_connected(client))
        {
            char *msg  = "{\"@message\":\"hello world\",\"@@id\":\"8f440041835117afc302a47965be727a\",\"@filehashkey\":\"c0419d1e720f6256fd44da6dbdacf5f7\",\"@collectiontime\":\"2023-01-31T09:59:13.827+08:00\",\"@hostname\":\"ck08\",\"@path\":\"/root/chenyc/test/dc/mave/probes/itoa-flow/data/utf-8.log\",\"@rownumber\":1,\"@seq\":1,\"@topic\":\"dc_test\",\"@ip\":\"192.168.110.8\",\"@taskid\":\"5961086096158208\"}";
            int i = 0;
            for (i = 0; i < client->config->batch; i++) {
                if (client->push(client, msg) == false){
                    printf("push message failed\n");
                    break;
                }
            }
            int n = client->send(client);
            if (n < 0) {
                client->stop(client);
                client = NULL;
                break;
            } else {
                int ack = client->wait_and_ack(client);
                metrics.ack_lines += ack;
                metrics.send_bytes += n;
                //printf("got ack: %d\n", ack);
            }
        }
    }

    if (client) {
        client->stop(client);
        client = NULL;
    }
    return 0;
}
