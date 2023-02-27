#include "../lumberjack.h"

int main(void){
    lumberjack_config_t config  = {
        .hosts                  = "192.168.110.8",
        .port                   = 7443,
        .compress_level         = 0,
        .client_port_range      = "60000,60100",
        .bandwidth              = 0,
        .protocol               = LUMBERJACK_PROTO_VERSION_V2,
        .timeout                = 10,
        .batch                  = 1,
        .with_ssl               = true,
        .metric_interval        = 5,
        .metric_enable          = true,
    };
    // lumberjack_config_t config = {0};
    lumberjack_client_t *client = lumberjack_new_client("itoa-flow-5961086096158208", &config);
    printf("client created %s:%d \n", client->conn.host, client->conn.port);
    client->start(client);
    printf("client started\n");
    while (1) {
        client->metrics_report(client);
        if (client->is_connected(client)) {
            char *msg  = "{\"@message\":\"hello world\",\"@@id\":\"8f440041835117afc302a47965be727a\",\"@filehashkey\":\"c0419d1e720f6256fd44da6dbdacf5f7\",\"@collectiontime\":\"2023-01-31T09:59:13.827+08:00\",\"@hostname\":\"ck08\",\"@path\":\"/root/chenyc/test/dc/mave/probes/itoa-flow/data/utf-8.log\",\"@rownumber\":1,\"@seq\":1,\"@topic\":\"dc_test\",\"@ip\":\"192.168.110.8\",\"@taskid\":\"5961086096158208\"}";
            while (client->push(client, msg))
                ;
            int n = client->send(client);
            if (n < 0) {
                printf("send message failed\n");
                break;
            } else {
                int ack = client->wait_and_ack(client);
                // printf("got ack: %d\n", ack);
				sleep(1);
            }
        }
    }

    if (client) {
        client->stop(client);
        client = NULL;
    }
    return 0;
}
