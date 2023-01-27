#ifndef LUMBERJACK_H_
#define LUMBERJACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#else
#include <windows.h>
#endif


#include "constant.h"

typedef struct lumberjack_config_t {
    char                *hosts;
    unsigned int        port;
    unsigned int        batch;
    boolean             with_ssl;
    char                protocol;
    unsigned int        compress_level;
    unsigned int        bandwidth;
    char                *client_port_range;
    int                 timeout;
        
    // externel variable
    boolean             _client_port_enable;
    int                 _client_port_start;
    int                 _client_port_end;
}lumberjack_config_t;


typedef union lumberjack_address_t {
    struct sockaddr_in  v4;         // ipv4
    struct sockaddr_in6 v6;         // ipv6
}lumberjack_address_t;

typedef struct lumberjack_connnect_t {
    char                    *host;
    int                     port;
    lumberjack_address_t    addr;
    int                     domain;
    int                     sock;
    int                     sock_status;
    boolean                 is_ipv6;
}lumberjack_connnect_t;


typedef struct lumberjack_data_t {
    int             wsize; // real size
    int             cap;   // each batch size
    char            **message;
    char            *data;
    unsigned int    size;
}lumberjack_data_t;

typedef struct lumberjack_header_t {
    char window[2];         // 2W
    char compress[2];       // 2C
    char data[2];           // 2J
    char ack[2];            // 2A
}lumberjack_header_t;


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
    void (*stop)(struct lumberjack_client_t *slef);

    /*-------------------------------------------------*/
    lumberjack_config_t *config;
    lumberjack_connnect_t  conn;
    char *hosts[LUMBERJACK_HOSTS_SIZE];
    int host_len;
    lumberjack_header_t *header;
    lumberjack_data_t *data;
}lumberjack_client_t;

lumberjack_client_t *lumberjack_new_client(lumberjack_config_t *config);

#ifdef __cplusplus
}
#endif

#endif