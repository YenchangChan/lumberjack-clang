#include "constant.h"
#include "utils.h"
#include "lumberjack.h"
#include <sys/types.h>

/* =========================================================================
 configuration
============================================================================*/
static void lumberjack_config_normalize(lumberjack_client_t *self, lumberjack_config_t *config) {
    if (config == NULL) return;

    config->hosts = (config->hosts == NULL)? "127.0.0.1" :config->hosts;
    if (config->with_ssl) {
        config->port = (config->port == 0)? LUMBERJACK_SSL_PORT_DEFAULT: config->port;
    } else {
        config->port = (config->port == 0)? LUMBERJACK_PORT_DEFAULT: config->port;
    }
    config->batch = (config->batch == 0)? LUMBERJACK_BATCH_DEFAULT:config->batch;
    config->protocol = (config->protocol == '\0') ? LUMBERJACK_PROTO_VERSION_V2 : config->protocol;

    if (config->client_port_range) {
        char *ports[2];
        int num = utils_string_split(config->client_port_range, ports, ",");
        config->_client_port_enable = (num == 2);
        if (config->_client_port_enable){
            int start, end;
            start = atoi(ports[0]);
            end = atoi(ports[1]);
            config->_client_port_start = (start < LUMBERJACK_MIN_PORT) ? LUMBERJACK_MIN_PORT : start;
            end = (end == 0) ? LUMBERJACK_MAX_PORT : end;
            config->_client_port_end = (end > LUMBERJACK_MAX_PORT) ? LUMBERJACK_MAX_PORT : end;
            config->_client_port_enable = (config->_client_port_start < config->_client_port_end);
        }
        for ( int i = 0; i < num; i++) {
            _FREE(ports[i]);
        }
    }

    self->config->hosts = strdup(config->hosts);
    self->config->port = config->port;
    self->config->batch = config->batch;
    self->config->bandwidth = config->bandwidth;
    self->config->timeout = config->timeout;
    self->config->compress_level = config->compress_level;
    self->config->protocol = config->protocol;
    self->config->with_ssl = config->with_ssl;
    self->config->_client_port_enable = config->_client_port_enable;
    self->config->_client_port_start = config->_client_port_start;
    self->config->_client_port_end = config->_client_port_end;
}

static void lumberjack_pick_client_port(lumberjack_client_t *self) {
    int port = 0;
    struct sockaddr_in client_addr;
    time_t t;
    char counter[65536] = {0};
    int sum = 0;
    int range = self->config->_client_port_end - self->config->_client_port_start;
    bzero((void*)&client_addr, sizeof(client_addr));
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_family = self->conn.domain;

    srand((unsigned)time(&t));
    while (sum < range) {
        port = rand() % (range + 1) + self->config->_client_port_start;
        if (counter[port - 1] == '\0') {
            counter[port - 1] = '1';
            sum++;
        } else if (counter[port - 1] == '1') {
            continue;
        }

        client_addr.sin_port = htons(port);
        if (bind(self->conn.sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) == 0) {
            break;
        }
    }
}

static char* lumberjack_pick_next_valid_host(char **hosts,int hosts_size, int from) {
    int i = from+1;
    while(i != from) {
        if (i >= hosts_size) i = i % hosts_size;
        if (hosts[i] != NULL && strlen(hosts[i]) != 0){
            return hosts[i];
        }
        i++;
    }
    return NULL;
}

static void lumberjack_repick_host(lumberjack_client_t *self) {
    int i=0;
    //random pick one if init
    if(self->conn.host == NULL) {
        srand(time(NULL));
        i = rand() % self->host_len;
        if (self->hosts[i] != NULL && strlen(self->hosts[i]) != 0){
            self->conn.host = self->hosts[i];
            self->conn.is_ipv6 = utils_is_ipv6_address(self->conn.host);
            return;
        }
    }

    //if host is not valid or host is exist
    i = 0;
    while ( i < self->host_len ) {
        if (strcasecmp(self->hosts[i], self->conn.host) == 0)
            break;
        i++;
    }

    self->conn.host = lumberjack_pick_next_valid_host(self->hosts, self->host_len, i);
    self->conn.is_ipv6 = utils_is_ipv6_address(self->conn.host);
    self->conn.domain = self->conn.is_ipv6 ? AF_INET6 : AF_INET;
}

/* =========================================================================
 protocol
============================================================================*/
static void lumberjack_build_header(lumberjack_config_t *config, lumberjack_header_t *header){
    header->window[0] = config->protocol;
    header->window[1] = LUMBERJACK_WINDOW;

    if (config->compress_level > 0) {
        header->compress[0] = config->protocol;
        header->compress[1] = LUMBERJACK_COMPRESS;
    } else {
        header->data[0] = config->protocol;
        header->data[1] = (config->protocol == LUMBERJACK_PROTO_VERSION_V1) ? LUMBERJACK_DATA : LUMBERJACK_JSON;
    }

    header->ack[0] = config->protocol;
    header->ack[1] = LUMBERJACK_ACK;
}

int lumberjack_parse_ack(lumberjack_header_t *header, char *data, int len){
    // 2A 
    int got_ack = -1;
    if (len >= 6 && memcmp(data, header->ack, 2) == 0) {
        memcpy(&got_ack, &data[2], sizeof(int));
        return ntohl(got_ack);
    }
    return got_ack;
}

void lumberjack_pack_message(lumberjack_client_t *self){
    int i, size = 0;
    char *single_data = NULL;
    char *data = NULL;
    int wsize = htonl(self->data->wsize);
    self->data->data = calloc(1, 6);
    memcpy(self->data->data, self->header->window, 2);
    memcpy(self->data->data + 2, (char *)&(wsize), sizeof(int));
    for (i = 0; i < self->data->wsize; i++){
        int n = 6 + strlen(self->data->message[i]);
        int seq = htonl(i + 1);
        size += n;
        single_data = calloc(1, n);
        memcpy(single_data, self->header->data, 2);
        memcpy(single_data + 2, (char *)&seq, sizeof(int));
        memcpy(single_data + 6, self->data->message[i], strlen(self->data->message[i]));
        data = realloc(data, size);
        memcpy(data + size - n, single_data, n);
        _FREE(single_data);
    }
    if (self->config->compress_level == 0) {
        self->data->data = realloc(self->data->data, 6 + size);
        memcpy(self->data->data + 6, data, size);
        self->data->size = size;
        _FREE(data);
    }
}


void lumberjack_reset_message(lumberjack_client_t *self){
    if (self->data->data) {
        _FREE(self->data->data);
        self->data->size = 0;
    }
    for (int i = 0; i < self->data->wsize; i++) {
        self->data->message[i] = NULL;
    }
    self->data->wsize = 0;
}

/* =========================================================================
 callback
============================================================================*/
static boolean on_is_connected(lumberjack_client_t *self){
    int rv = -1;
    switch (self->conn.sock_status) {
        case SS_DISCONNECT:
            break;
        case SS_PENDING:
            if (self->conn.is_ipv6) {
                rv = connect(self->conn.sock, (struct sockaddr*)&self->conn.addr.v6, sizeof(self->conn.addr.v6));
            } else {
                rv = connect(self->conn.sock, (struct sockaddr*)&self->conn.addr.v4, sizeof(self->conn.addr.v4));
            }
            if (rv ==  0 || errno == EISCONN){
                return true;
            }
            break;
        case SS_CONNECTED:
            return true;
        default:
            break;
    }
    
    //should reconnect
    lumberjack_repick_host(self);
    self->conn.sock_status = SS_DISCONNECT;
    self->start(self);
    return self->conn.sock_status == SS_CONNECTED;
}

static void on_start(struct lumberjack_client_t *self){
    if (self->conn.sock > 0) {
        close(self->conn.sock);
    }
    int rv = -1;
    self->conn.sock = socket(self->conn.domain, SOCK_STREAM, 0);
    if (self->conn.sock == -1){
        return;
    }
    if (self->conn.is_ipv6) {
        bzero((void*)&self->conn.addr.v6, sizeof(self->conn.addr.v6));
        self->conn.addr.v6.sin6_family = self->conn.domain;
        self->conn.addr.v6.sin6_port = htons(self->config->port);
        inet_pton(self->conn.domain, self->conn.host, &self->conn.addr.v6.sin6_addr);   
    } else {
        bzero((void*)&self->conn.addr.v4, sizeof(self->conn.addr.v4));
        self->conn.addr.v4.sin_family = self->conn.domain;
        self->conn.addr.v4.sin_port = htons(self->config->port);
        inet_pton(self->conn.domain, self->conn.host, &self->conn.addr.v4.sin_addr);   
    }
    if (self->config->_client_port_enable) {
        lumberjack_pick_client_port(self);
    }

    int flags = fcntl(self->conn.sock, F_GETFL, 0);
    fcntl(self->conn.sock, F_SETFL, flags|O_NONBLOCK);
    if (self->conn.is_ipv6) {
        rv = connect(self->conn.sock, (struct sockaddr*)&self->conn.addr.v6, sizeof(self->conn.addr.v6));
    } else {
        rv = connect(self->conn.sock, (struct sockaddr*)&self->conn.addr.v4, sizeof(self->conn.addr.v4));
    }
    if (rv == 0) {
        self->conn.sock_status = SS_CONNECTED;
        return;
    } else {
        if (errno == EINPROGRESS) {
            self->conn.sock_status = SS_PENDING;
        } else {
            perror("connect");
        }
    }
}

static boolean on_push(lumberjack_client_t *self, char *message) {
    if (self->data->wsize >= self->data->cap) return false;
    *(self->data->message + self->data->wsize) = message;
    (self->data->wsize)++;
    return true;
}

static int on_send(lumberjack_client_t *self){
    if (self->is_connected(self) == false) {
        return -1;
    }
    lumberjack_pack_message(self);
    unsigned int has_sended = 0;
    do {
        int n = 0;
        time_t before = utils_time_now();
        if (self->config->bandwidth > 0) {
            n = (self->data->size > self->config->bandwidth) ? self->config->bandwidth : self->data->size;
        } else {
            n = self->data->size;
        }
        int nbytes = send(self->conn.sock, self->data->data, n, 0);
        if (nbytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                nbytes  =  0;
            } else {
                self->conn.sock_status = SS_DISCONNECT;
                close(self->conn.sock);
                break;
            }
        }
        has_sended += nbytes;
        printf("haas_sended: %u, size: %u\n", has_sended, self->data->size);
        if (self->config->bandwidth > 0 && has_sended < self->data->size) {
            time_t now = utils_time_now();    
            if (now - before > 0) {
                utils_sleep(before + 1 * 1e6 - now);
            }
        }
    } while (has_sended < self->data->size);
    lumberjack_reset_message(self);
    return has_sended;
}

unsigned int on_wait_and_ack(lumberjack_client_t *self){
    if (self->is_connected(self) == false) {
        return 0;
    }
    char buffer[1024];
    int nbytes = recv(self->conn.sock, buffer,  1024, 0);
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        } else {
            self->conn.sock_status = SS_DISCONNECT;
            close(self->conn.sock);
        }
    }

    return lumberjack_parse_ack(self->header, buffer, nbytes);
}

static void on_stop(lumberjack_client_t *self){
    if (self->conn.sock > 0){
        close(self->conn.sock);
    }

    int i  = 0;
    for(i = 0; i <  self->host_len;  i++)  {
        if (self->hosts[i])  {
            _FREE(self->hosts[i]);
        }
    }
    _FREE(self->config->hosts);
    _FREE(self->config);
    _FREE(self->header);
    _FREE(self->data->message);
    _FREE(self->data);
    _FREE(self);
}


/* =========================================================================
 public expose
============================================================================*/
lumberjack_client_t *lumberjack_new_client(lumberjack_config_t *config){
    lumberjack_client_t *client = (lumberjack_client_t*)calloc(1, sizeof(lumberjack_client_t));
    if (client == NULL) {
        return client;
    }
    client->config = calloc(1, sizeof(lumberjack_config_t));
    lumberjack_config_normalize(client, config);
    client->host_len = utils_string_split(config->hosts, client->hosts, ","); //will alloc memory, but not free
    lumberjack_repick_host(client);
    client->conn.port = client->config->port;
    client->conn.sock_status = SS_DISCONNECT;
    client->conn.sock = -1;

    client->header = calloc(1, sizeof(lumberjack_header_t));
    lumberjack_build_header(client->config, client->header);
    client->data = calloc(1, sizeof(lumberjack_data_t));
    client->data->wsize = 0;
    client->data->cap = client->config->batch;
    client->data->message  = calloc(client->data->cap, sizeof(char*));

    client->start = on_start;
    client->is_connected = on_is_connected;
    client->push = on_push;
    client->send = on_send;
    client->wait_and_ack = on_wait_and_ack;
    client->stop = on_stop;
    return client;
}
