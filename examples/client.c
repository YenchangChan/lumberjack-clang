#include "../lumberjack.h"

int main(void){
    lumberjack_config_t config  = {
        .hosts = "127.0.0.1",
        .port  = 7070,
        .compress_level =  0,
        .client_port_range = "60000,60100",
        .bandwidth = 10485760,
        .protocol = LUMBERJACK_PROTO_VERSION_V2,
        .timeout = 10,
        .batch = 300,
    };
    //lumberjack_config_t config = {0};
    lumberjack_client_t *client = lumberjack_new_client(&config);
    printf("client created %s:%d \n", client->conn.host, client->conn.port);
    client->start(client);
    printf("client started %d\n", client->conn.sock_status);
    while (1) {
        if (client->is_connected(client)) {
            printf("client connected\n");
            char *msg  = "{\"@topic\":\"dc-test\",\"@message\":\"hello  world\"}";
            for (unsigned int i = 0; i < client->config->batch; i++) {
                if (client->push(client, msg)){
                    printf("send message[%u]: %s\n", i, msg);
                } else {
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
                printf("ack: %d\n", ack);
            }
        }
    }
    
    if (client) {
        client->stop(client);
        client = NULL;
    }
    return 0;
}