#ifndef DRCOM_H
#define DRCOM_H

#define PKT_REQUEST        0x01
#define PKT_CHALLENGE      0x02
#define PKT_LOGIN          0x0103
#define PKT_ACK_SUCCESS    0x04
#define PKT_ACK_FAILURE    0x05
#define PKT_LOGOUT         0x0106
#define PKT_KEEPALIVE      0x07
#define PKT_PING           0xff
#define PKT_PING_ACK       0x0107
#define PKT_MESSAGE        0x4d

#define ERROR_USERNAME_PASSWD_1          0x03
#define ERROR_USERNAME_PASSWD_2          0x15
#define ERROR_TIME_TRAFFIC_OVERFLOW      0x04
#define ERROR_ACCOUNT_DISABLE            0x05
#define ERROR_IP_STATIC                  0x17
#define ERROR_ADMIN_RESET                0x15

#define MODULE_FILE_MD5 "e7cb5e0f955dfd995d3e7c921bf80809"
#define MODULE_FILE_CHECKSUM "82bc935b"

#include <stdint.h>

#pragma pack(1)

typedef struct {
    uint32_t dwOSVersionInfoSize;
    uint32_t dwMajorVersion;
    uint32_t dwMinorVersion;
    uint32_t dwBuildNumber;
    uint32_t dwPlatformId;
    char drcom[6];
    uint32_t unk;
    uint8_t zero1[54];
    char module_md5[32];
    char module_checksum[8];
    uint8_t zero2[24];
} VersionInfo;

struct drcom_host_header
{
    uint16_t pkt_type;
    uint8_t zero;
    uint8_t len;
    uint8_t checksum[16];
};

struct drcom_serv_header
{
    uint8_t pkt_type;
    uint8_t seq;
    uint16_t timestamp;
};

struct drcom_request
{
    uint8_t pkt_type;
    uint8_t seq;
    uint16_t timestamp;
    uint8_t version;
    uint8_t zero[15];
};

struct drcom_challenge
{
    struct drcom_serv_header serv_header;
    uint32_t challenge;
    uint16_t encrypt_mode;
    uint16_t dwMinorVersion;
    uint16_t dwMajorVersion;
    uint16_t unk1;
    uint32_t zero1;
    uint32_t auth_host;
    uint32_t unk2;
    uint16_t zero2;
    uint32_t unk3;
    uint32_t zero3;
    uint16_t driver;
    uint16_t zero4;
    uint16_t sys_auth_opt;
    uint8_t zero5[32];
};

struct drcom_host
{
    char host_name[32];
    uint32_t dnsp;
    uint32_t dhcp;
    uint32_t dnss;
    uint32_t wins_sevrs[2];
    VersionInfo sys_info;
};

struct drcom_login
{
    struct drcom_host_header host_header;
    char user_name[36];
    uint8_t check_status;
    uint8_t mac_num;
    uint8_t mac_xor[6];
    uint8_t checksum1[16];
    uint8_t host_ip_num;
    uint32_t host_ip[4];
    uint8_t checksum2_half[8];
    uint8_t dog;
    uint8_t zero1[4];
    struct drcom_host host_info;
    uint8_t version;
    uint8_t unk_zero1;
    uint8_t magic_number[2];
    uint32_t checksum3;
    uint16_t unk_zero2;
    uint8_t mac_addr[6];
    uint16_t zero2;
    uint16_t checksum4;
};

struct drcom_auth
{
    char drco[4];
    uint32_t serv_ip;
    uint16_t serv_port;
    uint32_t host_ip;
    uint16_t host_port;
};

struct drcom_ack_header
{
    uint8_t pkt_type;
    uint16_t zero;
    uint8_t len;
    uint8_t error_code;
};

struct drcom_acknowledgement
{
    struct drcom_ack_header ack_header;
    uint32_t time_usage;
    uint32_t vol_usage;
    uint32_t balance;
    uint8_t unk1[6];
    struct drcom_auth auth_info;
    uint32_t unk2;
    uint8_t unk3;
    uint8_t antiproxy;
};

struct drcom_ping
{
    uint8_t pkt_type;
    uint8_t checksum[16];
    uint8_t zero[3];
    struct drcom_auth auth_info;
    uint16_t timestamp;
};

struct drcom_ping_ack_header
{
    uint16_t pkt_type;
    uint16_t len;
    uint8_t flag;
    uint8_t unk;
    uint16_t timestamp;
    uint32_t unk_time;
    uint32_t auth_host;
};

struct drcom_ping_ack
{
    struct drcom_ping_ack_header header;
    uint32_t unk1;
    uint32_t unk2;
    uint32_t zero1;
    uint16_t driver;
    uint16_t zero2;
    uint32_t time_usage;
    uint32_t vol_usage;
    uint32_t unk3;
    uint32_t zero3[3];
    uint32_t unk4;
    uint32_t ff;
};

struct drcom_keepalive
{
    uint8_t pkt_type;
    uint8_t seq;
    uint16_t len;
    uint8_t flag;
    uint8_t step;
    uint16_t driver;
    uint32_t timestamp;
    uint32_t flag2;
    uint32_t salt1;
    uint32_t salt2;
    uint32_t checksum;
    uint32_t auth_host;
    uint32_t zero1;
    uint32_t zero2;
};

struct drcom_message
{
    uint8_t pkt_type;
    uint8_t flag;/*0x150x26*/
    uint16_t unknown;
    char command[10];
    char detail[32];
};

struct drcom_logout
{
    struct drcom_host_header host_header;
    char user_name[36];
    uint8_t check_status;
    uint8_t mac_num;
    uint8_t mac_xor[6];
    struct drcom_auth auth_info;
};
#pragma pack()

#endif
