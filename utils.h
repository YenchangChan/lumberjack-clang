#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef _WIN32
#include <sys/time.h>
#else

#endif

#include "constant.h"

int utils_string_split(const char *src,  char  **dst, const char *delim);
boolean utils_is_ipv6_address(char *address);
int64_t utils_time_now();
void utils_sleep(int64_t t);
void utils_dump_hex(char *str, int len);
char *utils_fmt_size(int64_t size);

#ifdef __cplusplus
}
#endif

#endif /* __UTILS_H__ */