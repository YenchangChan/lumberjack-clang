#include "constant.h"
#include "utils.h"
#include "lumberjack.h"

/* =========================================================================
 configuration
============================================================================*/
static void lumberjack_config_normalize(lumberjack_client_t *self, lumberjack_config_t *config) {
    if (config == NULL) return;

    config->hosts = (config->hosts == NULL) ? "127.0.0.1" : config->hosts;
    if (config->with_ssl) {
        config->port = (config->port == 0)? LUMBERJACK_SSL_PORT_DEFAULT: config->port;
    } else {
        config->port = (config->port == 0)? LUMBERJACK_PORT_DEFAULT: config->port;
    }
    config->batch = (config->batch == 0)? LUMBERJACK_BATCH_DEFAULT:config->batch;
    config->protocol = (config->protocol == '\0') ? LUMBERJACK_PROTO_VERSION_V2 : config->protocol;
    // only support v2
    config->protocol = (config->protocol >= LUMBERJACK_PROTO_VERSION_MIN) ? LUMBERJACK_PROTO_VERSION_MIN : config->protocol;
    config->timeout = (config->timeout == 0) ? LUMBERJACK_TIMEOUT_DEFAULT : config->timeout;
    config->metric_interval = (config->metric_interval == 0) ? LUMBERJACK_METRIC_INTERVAL_DEFAULT : config->metric_interval;

    if (config->client_port_range) {
        char *ports[2];
        int i = 0;
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
        for (i = 0; i < num; i++) {
            _FREE(ports[i]);
        }
    }

    self->host_len = utils_string_split(config->hosts, self->hosts, ","); // will alloc memory, but not free

    self->config->hosts = strdup(config->hosts);
    self->config->port = config->port;
    self->config->batch = config->batch;
    self->config->bandwidth = config->bandwidth;
    self->config->timeout = config->timeout;
    self->config->metric_interval = config->metric_interval;
    self->config->metric_enable = config->metric_enable;
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
 ssl
============================================================================*/
#ifdef HAVE_SSL_H
void lumberjack_ssl_connect(lumberjack_connect_t *conn) {
    conn->ssl_handle = NULL;
    conn->ssl_ctx = NULL;

    if (conn->sock) {
        SSL_load_error_strings();
        SSL_library_init();
        OpenSSL_add_all_algorithms();

        conn->ssl_ctx = SSL_CTX_new(TLSv1_2_method());
        if (conn->ssl_ctx == NULL) {
            ERR_print_errors_fp(stderr);
        }

        conn->ssl_handle = SSL_new(conn->ssl_ctx);
        if (conn->ssl_handle == NULL) {
            ERR_print_errors_fp(stderr);
        }

        if (!SSL_set_fd(conn->ssl_handle, conn->sock)) {
            ERR_print_errors_fp(stderr);
        }
        if (SSL_connect(conn->ssl_handle) != 1) {
            ERR_print_errors_fp(stderr);
        }
        printf("ssl connection established\n");
    } else {
        printf("ssl connection failed\n");
    }
}

void lumberjack_ssl_disconnect(lumberjack_connect_t *conn)
{
    if (conn->ssl_handle) {
        SSL_shutdown(conn->ssl_handle);
        SSL_free(conn->ssl_handle);
    }

    if (conn->ssl_ctx) {
        SSL_CTX_free(conn->ssl_ctx);
    }
}

#endif
/* =========================================================================
 protocol
============================================================================*/

static void lumberjack_lock(pthread_mutex_t *mutex) {
    pthread_mutex_lock(mutex);
}

static void lumberjack_unlock(pthread_mutex_t *mutex) {
    pthread_mutex_unlock(mutex);
}

static void lumberjack_set_sock_status(lumberjack_client_t *self, int status) {
    lumberjack_lock(&self->mutex);
    self->conn.sock_status = status;
    lumberjack_unlock(&self->mutex);
}

static int lumberjack_get_sock_status(lumberjack_client_t *self)
{
    int status;
    lumberjack_lock(&self->mutex);
    status = self->conn.sock_status;
    lumberjack_unlock(&self->mutex);
    return status;
}

static void lumberjack_set_status(lumberjack_client_t *self, int status)
{
    lumberjack_lock(&self->mutex);
    self->status = status;
    lumberjack_unlock(&self->mutex);
}

static int lumberjack_get_status(lumberjack_client_t *self)
{
    int status;
    lumberjack_lock(&self->mutex);
    status = self->status;
    lumberjack_unlock(&self->mutex);
    return status;
}

static void lumberjack_build_header(lumberjack_config_t *config, lumberjack_header_t *header){
    header->window[0] = config->protocol;
    header->window[1] = LUMBERJACK_WINDOW;

    if (config->compress_level > 0) {
        header->compress[0] = config->protocol;
        header->compress[1] = LUMBERJACK_COMPRESS;
    }
    header->data[0] = config->protocol;
    header->data[1] = (config->protocol == LUMBERJACK_PROTO_VERSION_V1) ? LUMBERJACK_DATA : LUMBERJACK_JSON;
    
    header->ack[0] = config->protocol;
    header->ack[1] = LUMBERJACK_ACK;
}

static int lumberjack_parse_ack(lumberjack_header_t *header, char *data, int len){
    // 2A 
    int got_ack = -1;
    if (len >= 6 && memcmp(data, header->ack, 2) == 0) {
        memcpy(&got_ack, &data[2], sizeof(int));
        return ntohl(got_ack);
    }
    return got_ack;
}

static void lumberjack_compress_deflate(lumberjack_client_t *self, char *data, int len) {
#ifdef HAVE_ZLIB_H
    int ret;
    z_stream strm;
    unsigned char output[LUMBERJACK_CHUNK];
    int sp;
    int payload_size = 0;
    char *compressed = NULL;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    deflateInit(&strm, self->config->compress_level);
    strm.next_in = data;
    strm.avail_in = len;

    do
    {
        strm.avail_out = LUMBERJACK_CHUNK;
        strm.next_out = output;
        ret = deflate(&strm, Z_FINISH);
        //printf("ret:%d, strm.avail_out:%d, strm.avail_in: %d\n", ret, strm.avail_out, strm.avail_in);
        if (ret == Z_STREAM_ERROR)
        {
            break;
        }
        sp = payload_size;
        payload_size += (LUMBERJACK_CHUNK - strm.avail_out);
        compressed = realloc(compressed, payload_size);
        memcpy(compressed + sp, output, LUMBERJACK_CHUNK - strm.avail_out);
    }
    while (strm.avail_out == 0)
        ;

    deflateEnd(&strm);
    // 2C + payload_size(bigendian)
    self->data->size += (2 + sizeof(int) + payload_size);
    self->data->data = realloc(self->data->data, self->data->size);
    memcpy(self->data->data + 2 + sizeof(int), self->header->compress, 2);
    int compress_size = htonl(payload_size);
    memcpy(self->data->data + 4 + sizeof(int), &compress_size, sizeof(int));
    memcpy(self->data->data + 4 + 2 * sizeof(int), compressed, payload_size);
    _FREE(compressed);
#endif
}


static void lumberjack_pack_message(lumberjack_client_t *self){
    int i, size = 0;
    char *single_data = NULL;
    char *data = NULL;
    int wsize = htonl(self->data->wsize);
    self->data->size = 2 + sizeof(int);
    self->data->data = calloc(1, self->data->size);
    // 2W + window_size(bigendian)
    memcpy(self->data->data, self->header->window, 2);
    memcpy(self->data->data + 2, &(wsize), sizeof(int));
    for (i = 0; i < self->data->wsize; i++){
        // 2J + seq(bigendian) + payload_length(bigendian)
        int n = 2 + 2 * sizeof(int) + strlen(self->data->message[i]);
        int len = htonl(strlen(self->data->message[i]));
        int seq = htonl(i + 1);
        size += n;
        single_data = calloc(1, n);
        memcpy(single_data, self->header->data, 2);
        memcpy(single_data + 2, &seq, sizeof(int));
        memcpy(single_data + 2 + sizeof(int), &len, sizeof(int));
        memcpy(single_data + 2 + 2 * sizeof(int), self->data->message[i], strlen(self->data->message[i]));
        data = realloc(data, size);
        memcpy(data + size - n, single_data, n);
        _FREE(single_data);
    }

    if (self->config->compress_level > 0) {
        //utils_dump_hex(data, size);
        lumberjack_compress_deflate(self, data, size);
    } else {
        self->data->size += size;
        self->data->data = realloc(self->data->data, self->data->size);
        memcpy(self->data->data + 2 + sizeof(int), data, size);
    }
    _FREE(data);
}


static void lumberjack_reset_message(lumberjack_client_t *self){
    if (self->data->data) {
        _FREE(self->data->data);
        self->data->size = 0;
        self->data->has_sended = 0;
    }
    int i = 0;
    for (i = 0; i < self->data->wsize; i++) {
        self->data->message[i] = NULL;
    }
    self->data->sending = true;
    self->data->expect_ack = self->data->wsize;
    self->data->wsize = 0;
}

/* =========================================================================
 callback
============================================================================*/
static boolean on_is_connected(lumberjack_client_t *self){
    int rv = -1;
    switch (lumberjack_get_sock_status(self)) {
        case SS_DISCONNECT:
            break;
        case SS_PENDING:
            if (self->conn.is_ipv6) {
                rv = connect(self->conn.sock, (struct sockaddr*)&self->conn.addr.v6, sizeof(self->conn.addr.v6));
            } else {
                rv = connect(self->conn.sock, (struct sockaddr*)&self->conn.addr.v4, sizeof(self->conn.addr.v4));
            }
            if (rv ==  0 || errno == EISCONN){         
#ifdef HAVE_SSL_H
                if (self->config->with_ssl)
                {
                    lumberjack_ssl_connect(&self->conn);
                }
#endif
                FD_ZERO(&self->conn.readfds);
                FD_ZERO(&self->conn.writefds);
                FD_SET(self->conn.sock, &self->conn.readfds);
                FD_SET(self->conn.sock, &self->conn.writefds);
                lumberjack_set_sock_status(self, SS_CONNECTED);
                return true;
            }
            break;
        case SS_CONNECTED:
            return true;
        default:
            break;
    }
    
    //should reconnect
#ifdef HAVE_SSL_H
    if (self->config->with_ssl) {
        lumberjack_ssl_disconnect(&(self->conn));
    }
#endif
    lumberjack_repick_host(self);
    lumberjack_set_sock_status(self, SS_DISCONNECT);
    self->start(self);
    return lumberjack_get_sock_status(self) == SS_CONNECTED;
}

static void on_start(struct lumberjack_client_t *self){
    lumberjack_set_status(self, LJ_STATUS_START);
    if (self->conn.sock > 0) {
        close(self->conn.sock);
    }
    int rv = -1;
    printf("choose host %s:%d to connect\n", self->conn.host, self->conn.port);
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

    //set non-blocking
    int flags = fcntl(self->conn.sock, F_GETFL, 0);
    fcntl(self->conn.sock, F_SETFL, flags|O_NONBLOCK);
    if (self->conn.is_ipv6) {
        rv = connect(self->conn.sock, (struct sockaddr*)&self->conn.addr.v6, sizeof(self->conn.addr.v6));
    } else {
        rv = connect(self->conn.sock, (struct sockaddr*)&self->conn.addr.v4, sizeof(self->conn.addr.v4));
    }
    if (rv == 0) {   
#ifdef HAVE_SSL_H
        if (self->config->with_ssl)
        {
            lumberjack_ssl_connect(&self->conn);
        }
#endif
        FD_ZERO(&self->conn.readfds);
        FD_ZERO(&self->conn.writefds);
        FD_SET(self->conn.sock, &self->conn.readfds);
        FD_SET(self->conn.sock, &self->conn.writefds);
        lumberjack_set_sock_status(self, SS_CONNECTED);
        return;
    } else {
        if (errno == EINPROGRESS) {
            lumberjack_set_sock_status(self, SS_PENDING);
        } else {
            perror("connect");
        }
    }
}

static boolean on_push(lumberjack_client_t *self, char *message) {
    lumberjack_lock(&self->mutex);
    if (self->data->wsize >= self->data->cap){
        lumberjack_unlock(&self->mutex);
        return false;
    };
    
    if (self->config->bandwidth > 0) {
        // the same window data has not send complete, do not push
        time_t now = time(NULL);
        if (self->window.batch_is_sending && now  == self->window.time) {
            lumberjack_unlock(&self->mutex);
            return false;
        }
    }
    *(self->data->message + self->data->wsize) = message;
    (self->data->wsize)++;
    lumberjack_unlock(&self->mutex);
    return true;
}

static int on_send(lumberjack_client_t *self){
    if (self->is_connected(self) == false) {
        return -1;
    }
    lumberjack_lock(&self->mutex);
    if (self->data->wsize == 0) {
        // there is no data to send
        lumberjack_unlock(&self->mutex);
        return 0;
    }
    if (self->data->size == 0) {
        // last batch is all sended
        lumberjack_pack_message(self);
    }

    int n = 0;
    if (self->config->bandwidth > 0) {
        // at most get bandwidth size
        n = (self->data->size - self->data->has_sended > self->config->bandwidth) ? self->config->bandwidth : self->data->size - self->data->has_sended;

        time_t now = time(NULL);
        if (self->window.time == 0) {
            self->window.time = now;
            self->window.batch_is_sending = false;
        }

        if (now == self->window.time ) {
            if (self->window.batch_is_sending || self->window.count >= self->config->bandwidth) {
                // the same window, do not send duplicate
                //printf("batch_is_sending: %d, count: %d, bandwidth: %d\n", self->window.batch_is_sending, self->window.count, self->config->bandwidth);
                FD_SET(self->conn.sock, &self->conn.writefds);
                lumberjack_unlock(&self->mutex);
                return 0;
            }
        } else {
            // new window is beginning
            self->window.count = 0;
        }
        self->window.time = now;
    } else {
        n = self->data->size - self->data->has_sended;
    }

    int nbytes = 0;
    if (self->config->with_ssl) {
#ifdef HAVE_SSL_H
        nbytes = SSL_write(self->conn.ssl_handle, self->data->data + self->data->has_sended, n);
#endif
    } else {
        nbytes = send(self->conn.sock, self->data->data + self->data->has_sended, n, 0);
    }
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            nbytes  =  0;
        } else {
            self->conn.sock_status = SS_DISCONNECT;
            
#ifdef HAVE_SSL_H
            if (self->config->with_ssl) {
                lumberjack_ssl_disconnect(&(self->conn));
            }
#endif
            close(self->conn.sock);
            lumberjack_unlock(&self->mutex);
            return nbytes;
        }
    }
    self->data->has_sended += nbytes;
    self->metrics->send_bytes += nbytes;
    //printf("[%d][%d] has_sended %d, size: %d\n", time(NULL), self->window.time, self->data->has_sended, self->data->size);
    if ( self->data->has_sended == self->data->size) {
        // current batch is send completed
        lumberjack_reset_message(self);
        if (self->config->bandwidth > 0){
            self->window.count += nbytes;
            self->window.batch_is_sending = false;
        }
        
        FD_CLR(self->conn.sock, &(self->conn.writefds));
        FD_SET(self->conn.sock, &self->conn.readfds);
    } else {
        if (self->config->bandwidth > 0) {
            self->window.batch_is_sending = true;
            self->window.count += nbytes;
        }

        FD_SET(self->conn.sock, &self->conn.writefds);
    }
    lumberjack_unlock(&self->mutex);
    return nbytes;
}

unsigned int on_wait_and_ack(lumberjack_client_t *self){
    if (self->is_connected(self) == false) {
        return 0;
    }
    int nbytes = 0;
    char buffer[16] = {0};
    int64_t now, before;
    before = utils_time_now();
RETRY:
    now = utils_time_now();
    if (self->config->timeout > 0 && now - before >= self->config->timeout * 1e6) {
        // timeout
        return -1;
    }

    if (self->config->with_ssl) {
#ifdef HAVE_SSL_H
        nbytes = SSL_read(self->conn.ssl_handle, buffer, 16);
#endif
    } else {
        nbytes = recv(self->conn.sock, buffer, 16, 0);
    }
    //printf("nbytes: %d, buffer: %s\n", nbytes, buffer);
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            goto RETRY;
        } else {
            self->conn.sock_status = SS_DISCONNECT;
#ifdef HAVE_SSL_H
            if (self->config->with_ssl) {
                lumberjack_ssl_disconnect(&(self->conn));
            }
#endif
            close(self->conn.sock);
        }
    }

    int ack = lumberjack_parse_ack(self->header, buffer, nbytes);
    lumberjack_lock(&self->mutex);
    self->metrics->ack_lines += ack;
    lumberjack_unlock(&self->mutex);
    FD_CLR(self->conn.sock, &(self->conn.readfds));
    FD_SET(self->conn.sock, &self->conn.writefds);
    return ack;
}

static void on_stop(lumberjack_client_t *self){
    // 如果没有发完，要发完为止 
    if (self->worker_thread > 0) {
        pthread_join(self->worker_thread, NULL);
    }
    if (self->conn.sock > 0){
        close(self->conn.sock);
    }

    int i  = 0;
    for(i = 0; i <  self->host_len;  i++)  {
        _FREE(self->hosts[i]);
    }
    lumberjack_lock(&self->mutex);
    _FREE(self->config->hosts);
    _FREE(self->config);
    _FREE(self->header);
    _FREE(self->data->message);
    _FREE(self->data);
    _FREE(self->metrics->metric_name);
    _FREE(self->metrics);
    lumberjack_unlock(&self->mutex);
    pthread_mutex_destroy(&(self->mutex));
    _FREE(self);
}

static void on_metrics_report(lumberjack_client_t *self)
{
    if (self->config->metric_enable == false) {
        return;
    }
    time_t now = time(NULL);
	if (self->metrics->time == 0)
	{
		self->metrics->time = now;
	}
	else if (now - self->metrics->time >= self->metrics->interval)
	{
        lumberjack_lock(&self->mutex);
        printf("[metrics-logging] out_%s.timestamp=%d, out_%s.ack_lines=%ld, out_%s.send_bytes=%ld, out_%s.lines_per_second=%g, out_%s.size_per_second=%s/s\n",
               self->metrics->metric_name, time(NULL),
               self->metrics->metric_name, self->metrics->ack_lines,
               self->metrics->metric_name, self->metrics->send_bytes,
               self->metrics->metric_name, (self->metrics->ack_lines - self->metrics->prev_lines) / ((now - self->metrics->time) * 1.0),
               self->metrics->metric_name, utils_fmt_size((self->metrics->send_bytes - self->metrics->prev_bytes) / ((now - self->metrics->time))));
        self->metrics->time = now;
		self->metrics->prev_lines = self->metrics->ack_lines;
		self->metrics->prev_bytes = self->metrics->send_bytes;
        lumberjack_unlock(&self->mutex);
    }
}

static int on_event_type(lumberjack_client_t *self) {
    if (self->is_connected(self) == false){
        return LJ_EVENT_NONE;
    }
#ifndef _WIN32
    struct timeval tv = {0};
    tv.tv_sec = self->config->timeout;
#endif
    int nready = select(self->conn.sock + 1, &self->conn.readfds, &self->conn.writefds, NULL, &tv);
    if (nready == -1) {
        return LJ_EVENT_ERROR;
    }
    if (nready == 0) {
        return LJ_EVENT_NONE;
    }
    if (FD_ISSET(self->conn.sock, &self->conn.readfds))
    {
        return LJ_EVENT_READ;
    }
    if (FD_ISSET(self->conn.sock, &self->conn.writefds))
    {
        return LJ_EVENT_WRITE;
    }
    return LJ_EVENT_ERROR;
}

void lumberjack_event_loop(void *arg) {
    lumberjack_client_t *self = (lumberjack_client_t *)arg;
    int expect_ack = 0;
    while (1) {
		self->metrics_report(self);
		int event = self->event_type(self);
		if (event == LJ_EVENT_NONE) {
			continue;
		} else if (event == LJ_EVENT_ERROR) {
			break;
		} else if (event == LJ_EVENT_WRITE){
            self->send(self);
        } else if (event == LJ_EVENT_READ) {
            if (self->data->sending) {
                expect_ack = self->data->expect_ack;
            }
            int ack = self->wait_and_ack(self);
            if (expect_ack != ack) {
                // ack mismatched, throw this batch and retry
                printf("ack mismatched, expect %d but got %d\n", expect_ack, ack);
                lumberjack_set_status(self, LJ_STATUS_ACK_MISMATCH);
            } else {
                lumberjack_set_status(self, LJ_STATUS_START);
            }
            self->data->sending = false;
            self->data->expect_ack = 0;
            // printf("got ack = %d\n", ack);
        }
	}
}

static int on_bootstrap(lumberjack_client_t *self) {
    int rv = 0;
    if ((rv = pthread_create(&self->worker_thread, NULL, (void *)lumberjack_event_loop, (void *)self)) != 0) {
        return -1;
    }
}

static int on_status(lumberjack_client_t *self) {
    return lumberjack_get_status(self);
}

/* =========================================================================
 public expose
============================================================================*/
lumberjack_client_t *lumberjack_new_client(char *module_name, lumberjack_config_t *config) {
    lumberjack_client_t *client = (lumberjack_client_t *)calloc(1, sizeof(lumberjack_client_t));
    if (client == NULL) {
        return client;
    }
    client->config = calloc(1, sizeof(lumberjack_config_t));
    lumberjack_config_normalize(client, config);

    lumberjack_repick_host(client);
    client->conn.port = client->config->port;
    client->conn.sock_status = SS_DISCONNECT;
    client->conn.sock = -1;

    client->header = calloc(1, sizeof(lumberjack_header_t));
    lumberjack_build_header(client->config, client->header);
    client->data = calloc(1, sizeof(lumberjack_data_t));
    client->data->wsize = 0;
    client->data->cap = client->config->batch;
    client->data->size = 0;
    client->data->has_sended = 0;
    client->data->message = calloc(client->data->cap, sizeof(char *));

    client->window.count = 0;
    client->window.time = 0;
    client->window.batch_is_sending = false;

    client->metrics = calloc(1, sizeof(lumberjack_metrics_t));
    client->metrics->interval = config->metric_interval;
    module_name = (module_name == NULL) ? LUMBERJACK_METRIC_NAME_DEFAULT : module_name;
    client->metrics->metric_name = strdup(module_name);
    
    pthread_mutex_init(&(client->mutex), NULL);

    client->status = LJ_STATUS_INIT;

    client->start = on_start;
    client->is_connected = on_is_connected;
    client->push = on_push;
    client->send = on_send;
    client->wait_and_ack = on_wait_and_ack;
    client->stop = on_stop;
    client->metrics_report = on_metrics_report;
    client->event_type = on_event_type;
    client->bootstrap = on_bootstrap;
    client->get_status = on_status;
    return client;
}
