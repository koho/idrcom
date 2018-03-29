#include "build.h"
#include "md5.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>

struct drcom_request* build_request(struct drcom_request *pkt, uint8_t req_num) {
    memset(pkt, 0, sizeof(struct drcom_request));
    pkt->pkt_type = PKT_REQUEST;
    pkt->seq = req_num;
    pkt->timestamp = time(NULL);
    pkt->version = 0x2b;
    return pkt;
}

struct drcom_login* build_login(struct drcom_login *login_packet, struct Config *config, uint32_t challenge) {
    memset(login_packet, 0, sizeof(struct drcom_login));
    unsigned char buf1[22], md5buf[16];
    char buf2[25];
    int passwd_len, i;
    
    /* Fill the packet header */
    login_packet->host_header.pkt_type = PKT_LOGIN;
    login_packet->host_header.zero = 0;
    login_packet->host_header.len = strlen(config->user_name) + sizeof(struct drcom_host_header);
    
    /* Copy data to a buf for encryption */
    /* Checksum */
    memset(buf1, 0, sizeof(buf1));
    *(uint16_t *)buf1 = login_packet->host_header.pkt_type;
    *(uint32_t *)(buf1 + 2) = challenge;
    passwd_len = strlen(config->password);
    strncpy((char *)(buf1 + 6), config->password, sizeof(config->password));
    MD5((unsigned char *)buf1, passwd_len + 6, md5buf);
    memcpy(login_packet->host_header.checksum, md5buf, sizeof(login_packet->host_header.checksum));
    
    /* Copy username */
    strncpy(login_packet->user_name, config->user_name, sizeof(config->user_name));
    
    /* Since these information is not important, we use fixed number */
    login_packet->check_status = 0x20;
    
    login_packet->mac_num = 1;
    
    /* Mac xor checksum */
    memcpy(login_packet->mac_xor, config->mac, sizeof(config->mac));
    for (i = 0; i < sizeof(login_packet->mac_xor); ++i)
        login_packet->mac_xor[i] ^= login_packet->host_header.checksum[i];
    
    /* Checksum1 */
    memset(buf2, 0, sizeof(buf2));
    buf2[0] = 0x01;
    memcpy(buf2 + 1, config->password, passwd_len);
    *(uint32_t *)(buf2 + 1 + passwd_len) = challenge;
    MD5((unsigned char *)buf2, 1 + passwd_len + 4 + 4, md5buf);
    memcpy(login_packet->checksum1, md5buf, 16);
    
    /* Host IP information */
    login_packet->host_ip_num = 1;
    login_packet->host_ip[0] = config->host_ip;
    
    /* wtf! second checksum */
    login_packet->checksum2_half[0] = 0x14;
    login_packet->checksum2_half[1] = 0x00;
    login_packet->checksum2_half[2] = 0x07;
    login_packet->checksum2_half[3] = 0x0b;
    MD5((unsigned char *)login_packet, 0x65, md5buf);
    /* Half of the md5 value is needed */
    memcpy(login_packet->checksum2_half, md5buf, sizeof(login_packet->checksum2_half));
    
    /* we've got a dog */
    login_packet->dog = 1;
    
    /* Host info */
    strncpy(login_packet->host_info.host_name, config->host_name, sizeof(config->host_name));
    login_packet->host_info.dnsp = config->dnsp;
    login_packet->host_info.dhcp = config->dhcp;
    login_packet->host_info.dnss = config->dnss;
    login_packet->host_info.wins_sevrs[0] = 0;
    login_packet->host_info.wins_sevrs[1] = 0;
    
    /* Just some fake information */
    login_packet->host_info.sys_info.dwOSVersionInfoSize = sizeof(VersionInfo);
    login_packet->host_info.sys_info.dwMajorVersion = 0x06;
    login_packet->host_info.sys_info.dwMinorVersion = 0x02;
    login_packet->host_info.sys_info.dwBuildNumber = 0x23f0;
    login_packet->host_info.sys_info.dwPlatformId = 0x02;
    
    /* Module information */
    strcpy(login_packet->host_info.sys_info.drcom, "DrCOM");
    login_packet->host_info.sys_info.unk = 0x002b07cf;
    memcpy(login_packet->host_info.sys_info.module_md5, MODULE_FILE_MD5, strlen(MODULE_FILE_MD5));
    memcpy(login_packet->host_info.sys_info.module_checksum, MODULE_FILE_CHECKSUM, strlen(MODULE_FILE_CHECKSUM));
    
    /* Version */
    login_packet->version = 0x2b;
    
    /* Mac address */
    memcpy(login_packet->mac_addr, config->mac, 6);
    
    /* Magic number */
    login_packet->magic_number[0] = 2;
    login_packet->magic_number[1] = 0x0c;
    login_packet->checksum3 = 0x11072601;
    
    /* Checksum3 */
    uint32_t length = sizeof(struct drcom_login) / 4, checksum3 = 0x4d2;
    for (int i = 0; i < length; i++)
        checksum3 ^= ((uint32_t *)login_packet)[i];
    login_packet->checksum3 = checksum3 * 0x7b0;
    
    /* Checksum4 */
    length = (sizeof(struct drcom_login) - 2) / 2;
    uint16_t checksum4 = 0;
    for (int i = 0; i < length; i++)
    {
        checksum4 ^= ((uint16_t *)login_packet)[i];
    }
    login_packet->checksum4 = checksum4 * 0x2c7;
    
    return login_packet;
}

struct drcom_ping* build_ping(struct drcom_ping *ping, uint8_t *checksum, struct drcom_auth *auth_info) {
    memset(ping, 0, sizeof(struct drcom_ping));
    ping->pkt_type = PKT_PING;
    memcpy(ping->checksum, checksum, sizeof(ping->checksum));
    memcpy(&ping->auth_info, auth_info, sizeof(struct drcom_auth));
    ping->timestamp = time(NULL);
    return ping;
}

struct drcom_keepalive* build_keepalive(struct drcom_keepalive* keepalive, uint8_t seq, uint8_t step,
                                        uint16_t driver, uint32_t salt, uint32_t host) {
    memset(keepalive, 0, sizeof(struct drcom_keepalive));
    keepalive->pkt_type = PKT_KEEPALIVE;
    keepalive->seq = seq;
    keepalive->len = sizeof(struct drcom_keepalive);
    keepalive->flag = 0x0b;
    keepalive->step = step;
    keepalive->driver = driver;
    keepalive->timestamp = time(0);
    keepalive->salt1 = salt;
    if (step == 3) {
        uint32_t checksum = 0, length = sizeof(struct drcom_keepalive) / 2;
        for (int i = 0; i < length; i++)
            checksum ^= ((uint16_t *)keepalive)[i];
        keepalive->checksum = checksum * 0x2c7;
        keepalive->auth_host = host;
    }
    return keepalive;
}

struct drcom_logout* build_logout(struct drcom_logout* logout_packet, struct Config *config, uint32_t challenge, struct drcom_auth *auth_info) {
    memset(logout_packet, 0, sizeof(struct drcom_logout));
    unsigned char buf1[22], md5buf[16];
    int passwd_len;
    
    /* Fill the packet header */
    logout_packet->host_header.pkt_type = PKT_LOGOUT;
    logout_packet->host_header.zero = 0;
    logout_packet->host_header.len = strlen(config->user_name) + sizeof(struct drcom_host_header);
    
    /* Copy data to a buf for encryption */
    /* Checksum */
    memset(buf1, 0, sizeof(buf1));
    *(uint16_t *)buf1 = logout_packet->host_header.pkt_type;
    *(uint32_t *)(buf1 + 2) = challenge;
    passwd_len = strlen(config->password);
    strncpy((char *)(buf1 + 6), config->password, sizeof(config->password));
    MD5((unsigned char *)buf1, passwd_len + 6, md5buf);
    memcpy(logout_packet->host_header.checksum, md5buf, sizeof(logout_packet->host_header.checksum));
    
    /* Copy username */
    strncpy(logout_packet->user_name, config->user_name, sizeof(config->user_name));
    
    /* Since these information is not important, we use fixed number */
    logout_packet->check_status = 0x20;
    
    logout_packet->mac_num = 1;
    
    /* Mac xor checksum */
    memcpy(logout_packet->mac_xor, config->mac, sizeof(config->mac));
    for (int i = 0; i < sizeof(logout_packet->mac_xor); ++i)
        logout_packet->mac_xor[i] ^= logout_packet->host_header.checksum[i];
    memcpy(&logout_packet->auth_info, auth_info, sizeof(struct drcom_auth));
    return logout_packet;
}
