#ifndef BUILD_H
#define BUILD_H

#include "common/drcom.h"
#include "common/config.h"

struct drcom_request* build_request(struct drcom_request *pkt, uint8_t req_num);
struct drcom_login* build_login(struct drcom_login *login_packet, struct Config *config, uint32_t challenge);
struct drcom_ping* build_ping(struct drcom_ping *ping, uint8_t *checksum, struct drcom_auth *auth_info);
struct drcom_keepalive* build_keepalive(struct drcom_keepalive* keepalive, uint8_t seq, uint8_t step,
                                        uint16_t driver, uint32_t salt, uint32_t host);
struct drcom_logout* build_logout(struct drcom_logout* logout_packet, struct Config *config, uint32_t challenge, struct drcom_auth *auth_info);

#endif
