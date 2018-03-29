#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_CONFIG_FILE "/etc/idrcom.conf"
#include <stdint.h>

struct Config {
    char user_name[36];
    char password[16];
    char host_name[32];
    char interface[16];
    uint8_t mac[6];
    uint32_t dhcp;
    uint32_t dnsp;
    uint32_t dnss;
    uint32_t host_ip;
    uint16_t host_port;
    uint32_t serv_ip;
    uint16_t serv_port;
    char host_ip_str[16];
};

int read_config(struct Config *config, const char *path);
int get_interface_info(struct Config *config);
bool ensure_config(struct Config *config);

#endif
