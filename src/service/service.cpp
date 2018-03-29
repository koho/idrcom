#include "service.h"
#include "common/log.h"
#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <arpa/inet.h>

Service::Service(Config *config) : connection(config->host_ip, config->host_port, config->serv_ip, config->serv_port) {
    this->config = config;
    this->logged_in = false;
    this->step = STEP_REQ_LOGIN;
    this->request_count = 1;
    this->last_recv_ping = 0;
    this->last_sent_ping = 0;
    this->last_recv_keepalive = 0;
    this->keepalive_sent_count = 0;
    this->keepalive_step = 1;
    this->keepalive_driver = 0x270f;
    this->keepalive_salt = 0;
}

void Service::reset() {
    this->logged_in = false;
    this->step = STEP_REQ_LOGIN;
    this->last_recv_ping = 0;
    this->last_sent_ping = 0;
    this->keepalive_step = 1;
}

ssize_t Service::send_request() {
    request_count++;
    struct drcom_request request;
    build_request(&request, request_count);
    this->request_timestamp = request.timestamp;
    Log::i("request: %s, count %d", step_desc[step], request_count);
    return connection.send((char *)&request, sizeof(struct drcom_request));
}

ssize_t Service::send_login() {
    struct drcom_login login;
    build_login(&login, config, challenge);
    memcpy(this->checksum, login.host_header.checksum, sizeof(this->checksum));
    Log::i("login: user %s, IP %s", config->user_name, config->host_ip_str);
    return connection.send((char *)&login, sizeof(struct drcom_login));
}

ssize_t Service::send_ping() {
    struct drcom_ping ping;
    build_ping(&ping, checksum, &auth_info);
    ping_timestamp = ping.timestamp;
    Log::i("ping: ack %ds ago, last %ds ago",
           last_recv_ping == 0 ? 0 : (time(NULL) - last_recv_ping),
           last_sent_ping == 0 ? 0 : (time(NULL) - last_sent_ping));
    last_sent_ping = time(NULL);
    return connection.send((char *)&ping, sizeof(struct drcom_ping));
}

ssize_t Service::send_keepalive() {
    struct drcom_keepalive keepalive;
    build_keepalive(&keepalive, keepalive_sent_count, keepalive_step, keepalive_driver, keepalive_salt, config->host_ip);
    keepalive_timestamp = keepalive.timestamp;
    Log::i("keepalive: %s, ack %ds ago, count %d", step_desc[step],
           last_recv_keepalive == 0 ? 0 : (time(NULL) - last_recv_keepalive),
           keepalive_sent_count);
    return connection.send((char *)&keepalive, sizeof(struct drcom_keepalive));
}

ssize_t Service::send_logout() {
    struct drcom_logout logout;
    Log::i("logout: user %s, IP %s", config->user_name, config->host_ip_str);
    return connection.send((char *)build_logout(&logout, config, challenge, &auth_info), sizeof(struct drcom_logout));
}

void Service::set_auth(struct drcom_auth *auth_info) {
    memcpy(&this->auth_info, auth_info, sizeof(struct drcom_auth));
}

void Service::close() {
    connection._close();
}
