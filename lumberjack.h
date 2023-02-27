#ifndef LUMBERJACK_H_
#define LUMBERJACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>

#ifndef _WIN32
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#else
#ifndef _WIN32_WCE
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#else
#include <winsock.h>
#endif
#include <windows.h>
#endif

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

#ifdef HAVE_SSL_H
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "constant.h"
#include "buf.h"

typedef struct lumberjack_config_t {
    char                *hosts;     // only contians ip
    char                *endpoint;  // contains ip and port, for ipv4 ip:port and for ipv6 [ip]:port
    unsigned int        port;       // works with hosts, if endpoint is not null, then port will useless
    unsigned int        batch;
    boolean             with_ssl;
    char                *cafile;
    char                *certfile;
    char                *certkeyfile;
    char                protocol;   // only support v2 yet
    unsigned int        compress_level; // if level = 0, means no compress
    unsigned int        bandwidth; // if bandwidth = 0, means unlimit
    char                *client_port_range;
    int                 timeout;
    int                 metric_interval;
    boolean             metric_enable;

    // externel variable
    boolean             _client_port_enable;
    int                 _client_port_start;
    int                 _client_port_end;
}lumberjack_config_t;


typedef union lumberjack_address_t {
    struct sockaddr_in  v4;         // ipv4
    struct sockaddr_in6 v6;         // ipv6
}lumberjack_address_t;

typedef struct lumberjack_connect_t {
    char                    *host;
    int                     port;
    lumberjack_address_t    addr;
    int                     domain;
    int                     sock_status;
    boolean                 is_ipv6;
    fd_set                  readfds;
    fd_set                  writefds;
#ifndef _WIN32
    int                     sock;
#else
    SOCKET                  sock;
#endif
#ifdef HAVE_SSL_H
    SSL                     *ssl_handle;
    SSL_CTX                 *ssl_ctx;
#endif
}lumberjack_connect_t;


// data is each batch of messages
typedef struct lumberjack_data_t {
    int             wsize; // real size
    int             cap;   // each batch size
    char            **message; // array of message
    lumberjack_buf_t *buf;
    unsigned int    has_sended;
    boolean         sending;
    int             expect_ack;
} lumberjack_data_t;

typedef struct lumberjack_header_t {
    char window[2];         // 2W
    char compress[2];       // 2C
    char data[2];           // 2J
    char ack[2];            // 2A
}lumberjack_header_t;

typedef struct lumberjack_metrics_t {
    char *metric_name;
    time_t time;
    uint64_t prev_lines;
    uint64_t ack_lines;
    uint64_t prev_bytes;
    uint64_t send_bytes;
    int interval;
} lumberjack_metrics_t;

typedef struct lumberjack_window_t {
    time_t time;
    int64_t count;
    boolean batch_is_sending;
} lumberjack_window_t;

typedef struct lumberjack_client_t {

    // start a connection to connect server
    void (*start)(struct lumberjack_client_t *self);

    // if connected, return; else repick a host to reconnect
    boolean (*is_connected)(struct lumberjack_client_t *self);

    // push back data to batch messages
    boolean (*push)(struct lumberjack_client_t *self, char *message);
    
    // send a batch
    int (*send)(struct lumberjack_client_t *self);
    
    // recv a response and parse it to ack
    unsigned int (*wait_and_ack)(struct lumberjack_client_t *self);
    
    // close the connection and free memory
    void (*stop)(struct lumberjack_client_t *self);

    // report metrics to stdout
    void (*metrics_report)(struct lumberjack_client_t *self);

    int (*event_type)(struct lumberjack_client_t *self);

    int (*bootstrap)(struct lumberjack_client_t *self);

    int (*get_status)(struct lumberjack_client_t *self);

    /*-------------------------------------------------*/
    lumberjack_config_t *config;
    lumberjack_connect_t  conn;
    char *hosts[LUMBERJACK_HOSTS_SIZE];
    int host_len;
    lumberjack_header_t *header;
    lumberjack_data_t *data;
    lumberjack_metrics_t *metrics;
    lumberjack_window_t window;
    int status;
#ifndef _WIN32
    pthread_mutex_t mutex;
    pthread_t worker_thread;
#else
    HANDLE    mutex;
    HANDLE    worker_thread;
#endif
} lumberjack_client_t;

lumberjack_client_t *lumberjack_new_client(char *module_name, lumberjack_config_t *config);

#ifdef __cplusplus
}
#endif

#endif
