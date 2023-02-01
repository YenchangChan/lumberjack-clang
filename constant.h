#ifndef LUMBERJACK_CONSTANT_H_
#define LUMBERJACK_CONSTANT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int boolean;
#define true  1
#define false 0

#define LUMBERJACK_HOSTS_SIZE               128
#define LUMBERJACK_CHUNK                    2048

#define LUMBERJACK_BATCH_DEFAULT            300
#define LUMBERJACK_PORT_DEFAULT             7070
#define LUMBERJACK_SSL_PORT_DEFAULT         7443
#define LUMBERJACK_COMPRESS_LEVEL_DEFAULT   0
#define LUMBERJACK_TIMEOUT_DEFAULT          10
#define LUMBERJACK_PROTO_VERSION_V1         '1'
#define LUMBERJACK_PROTO_VERSION_V2         '2'
#define LUMBERJACK_PROTO_VERSION_MIN        '2'

#define SS_DISCONNECT  0
#define SS_PENDING     1
#define SS_CONNECTED   2

#define LUMBERJACK_MIN_PORT  0
#define LUMBERJACK_MAX_PORT  65535

#define LUMBERJACK_ACK            '\x41'       // 'A'
#define LUMBERJACK_WINDOW         '\x57'       // 'W'
#define LUMBERJACK_COMPRESS       '\x43'       // 'C'
#define LUMBERJACK_JSON           '\x4a'       // 'J'
#define LUMBERJACK_DATA           '\x44'       // 'D'


#define _FREE(v)    if (v != NULL) {     \
                        free(v);         \
                        v = NULL;        \
                    }

#ifdef __cplusplus
}
#endif


#endif