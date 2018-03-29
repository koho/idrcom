#include "config.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <errno.h>
#include <ifaddrs.h>

#ifdef __unix__
#include <linux/if_packet.h>
#define AF_LINK AF_PACKET
#elif __APPLE__
#include <net/if_dl.h>
#endif

int read_config(Config *config, const char *path) {
    FILE *config_file = fopen(path, "r");
    if (config_file == NULL) {
        Log::e("io: fail to open %s, %s", path, strerror(errno));
        return 1;
    }
    memset(config, 0, sizeof(struct Config));
    char option[100], key[20], value1[40], value2[40];
    while (!feof(config_file)) {
        memset(option, 0, sizeof(option));
        memset(key, 0, sizeof(key));
        memset(value1, 0, sizeof(value1));
        memset(value2, 0, sizeof(value2));
        fgets(option, sizeof(option), config_file);
        sscanf(option, "%s%s%s", key, value1, value2);
        if (strcmp(key, "user") == 0) {
            strncpy(config->user_name, value1, sizeof(config->user_name) - 1);
            strncpy(config->password, value2, sizeof(config->password) - 1);
        }
        if (strcmp(key, "server") == 0) {
            config->serv_ip = inet_addr(value1);
            config->serv_port = atoi(value2);
        }
        if (strcmp(key, "dns") == 0) {
            config->dnsp = inet_addr(value1);
            config->dnss = inet_addr(value2);
        }
        if (strcmp(key, "password") == 0)
            strncpy(config->password, value1, sizeof(config->password) - 1);
        if (strcmp(key, "interface") == 0)
            strncpy(config->interface, value1, sizeof(config->interface) - 1);
        if (strcmp(key, "port") == 0)
            config->serv_port = atoi(value1);
        if (strcmp(key, "listen") == 0)
            config->host_port = atoi(value1);
        if (strcmp(key, "hostname") == 0)
            strncpy(config->host_name, value1, sizeof(config->host_name) - 1);
        if (strcmp(key, "dnss") == 0)
            config->dnss = inet_addr(value1);
        if (strcmp(key, "dhcp") == 0)
            config->dhcp = inet_addr(value1);
    }
    fclose(config_file);
    return 0;
}

int get_interface_info(struct Config *config) {
    struct ifaddrs *if_list;
    if (getifaddrs(&if_list) == -1) {
        Log::e("getifaddrs: %s", strerror(errno));
        return 1;
    }
    bool found = false;
    for (struct ifaddrs *interface = if_list; interface; interface = interface->ifa_next) {
        if (strcmp(interface->ifa_name, config->interface) == 0) {
            found = true;
            if (interface->ifa_addr == NULL)
                continue;
            if (interface->ifa_addr->sa_family == AF_LINK) {
#ifdef __APPLE__
                sockaddr_dl *sdl = (sockaddr_dl *)interface->ifa_addr;
                memcpy(config->mac, LLADDR(sdl), sizeof(config->mac));
#elif __unix__
                struct sockaddr_ll * sll = (struct sockaddr_ll *)interface->ifa_addr;
                memcpy(config->mac, sll->sll_addr, sizeof(config->mac));
#endif
            } else if (interface->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in *addr = (struct sockaddr_in *)interface->ifa_addr;
                config->host_ip = addr->sin_addr.s_addr;
                strncpy(config->host_ip_str, inet_ntoa(addr->sin_addr), sizeof(config->host_ip_str));
            }
        }
    }
    freeifaddrs(if_list);
    if (!found) {
        Log::e("if: no interface named %s", config->interface);
        return 1;
    }
    if (config->host_ip == 0) {
        Log::e("if: %s has no IP configuration", config->interface);
        return 1;
    }
    if (strlen(config->host_name) == 0) {
        if (gethostname(config->host_name, sizeof(config->host_name)) == -1) {
            Log::e("gethostname: %s", strerror(errno));
            return 1;
        }
    }
    if (config->host_port == 0)
        config->host_port = config->serv_port;
    return 0;
}

bool ensure_config(struct Config *config) {
    if (strlen(config->user_name) == 0 || strlen(config->password) == 0) {
        Log::e("config: empty username or password");
        return false;
    }
    if (strlen(config->interface) == 0) {
        Log::e("config: interface is not specified");
        return false;
    }
    if (config->serv_ip == 0 || config->serv_port == 0) {
        Log::e("config: server IP or port is not configured");
        return false;
    }
    return true;
}
