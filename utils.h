#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#ifndef _WIN32_WCE
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#else
#include <winsock.h>
#endif
#include <windows.h>
#endif

#include "constant.h"

int utils_string_split(const char *src,  char  **dst, const char *delim);
boolean utils_is_ipv6_address(char *address);
void utils_dump_hex(char *str, int len);
char *utils_fmt_size(int64_t size);
int utils_split_host_port(const char *endpoint, char *host, int *port);

#ifdef __cplusplus
}
#endif

#endif /* __UTILS_H__ */
