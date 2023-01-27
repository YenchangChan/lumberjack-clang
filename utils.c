#include "utils.h"


int utils_string_split(const char *src,  char  **dst, const char *delim){
    if (src == NULL) return 0;
    if (dst == NULL) return 0;

    int i = 0;
    char *token;
    char *p = strdup(src);
    if (p){
        token = strtok(p, delim);
        while(token != NULL){
            dst[i] = strdup(token); 
            i++; 
            token = strtok(NULL, delim);
        }
    }
    return i;
}


boolean utils_is_ipv6_address(char *address){
    char *tmp;
    if (address == NULL) return false;
    tmp = strchr(address, ':');
    return tmp != NULL;
}

int64_t utils_time_now() {
#ifndef _WIN32
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e6 + tv.tv_usec; 
#else 
    int64_t tv = 0;
    FILETIME time;
#ifndef _WIN32_WCE
    GetSystemTimeAsFileTime(&time);
#else
    SYSTEMTIME st; 
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &time);
#endif
    FileTimeToAprTime(&tv, &time);
    return tv;
#endif
}

void utils_sleep(int64_t t) {
#ifndef _WIN32
#ifdef OS2
    DosSleep(t/1000);
#elif defined(BEOS)
    snooze(t);
#elif defined(NETWARE)
    delay(t/1000);
#else
    struct timeval tv; 
    tv.tv_usec = t % (int64_t)1e6;
    tv.tv_sec = t / 1e6;
    select(0, NULL, NULL, NULL, &tv);
#endif
#else   
    Sleep(DWORD)(t / 1000);
#endif
}