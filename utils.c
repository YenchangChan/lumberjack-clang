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

void utils_dump_hex(char *str, int len) {
    int i = 0;
    for (i = 0; i < len; i += 16)
    {
        int j = 0;
        int k = i;
        printf("%07x: ", i);
        for (j = 0; j < 16; j++)
        {
            if (k >= len) {
                printf("   ");
            } else {
                printf("%02x ", str[k++]);
            }
        }
        printf("   ");
        k = i;
        for (j = 0; j < 16; j++)
        {
            if (k >= len) {
                break;
            }
            if (str[k] > 31 && str[k] < 127){
                printf("%c", str[k++]);
            }else {
                printf(".");
                k++;
            }
        }
        printf("\n");
    }
}

char * utils_fmt_size(int64_t size) {
    static char buff[32] = {0};
    if (size < KB) {
        sprintf(buff, "%g B", size/1.0);
    } else if (size < MB) {
        sprintf(buff, "%g KB", size/(KB*1.0));
    } else if (size < GB){
        sprintf(buff, "%g MB", size / (MB * 1.0));
    } else {
        sprintf(buff, "%g GB", size / (GB * 1.0));
    }
    return buff;
}


int utils_split_host_port(const char *endpoint, char *host, int *port){
    int i = 0, split = 0, len = 0;
    char port_str[8] = {0};
    boolean is_ipv6 = (endpoint[0] == '[') ? true : false;
    len = strlen(endpoint);
    for (i = len; i > 0; i--) {
        if (*(endpoint + i) == ':') {
            split = i;
            break;
        }
    }
    if (split == 0 || split == len) {
        return -1;
    }
    if(is_ipv6) {
        if (split <= 2) {
            return -1;
        }
        strncpy(host, endpoint + 1, split - 2);
    } else {
        strncpy(host, endpoint, split);
    }
    
    strncpy(port_str, endpoint + i + 1, len - split);
    *port = atoi(port_str);
    if (*port < 0) {
        return -1;
    }
    return 0;
}
