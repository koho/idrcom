#ifndef SERVICE_H
#define SERVICE_H

#include "connection.h"
#include "core/build.h"

typedef enum {
    STEP_REQ_LOGIN,
    STEP_SEND_LOGIN,
    STEP_KEEPALIVE_1_2,
    STEP_KEEPALIVE_3_4,
    STEP_REQ_LOGOUT,
    STEP_SEND_LOGOUT
} Step;

const char *const step_desc[] = {
    "STEP_REQ_LOGIN",
    "STEP_SEND_LOGIN",
    "STEP_KEEPALIVE_1_2",
    "STEP_KEEPALIVE_3_4",
    "STEP_REQ_LOGOUT",
    "STEP_SEND_LOGOUT"
};

class Service {
public:
    Service(Config *config);
    Step step;
    Connection connection;
    // Interfaces for sender
    ssize_t send_request();
    ssize_t send_login();
    ssize_t send_logout();
    ssize_t send_ping();
    ssize_t send_keepalive();
    // Interfaces for receiver
    bool logged_in;
    // Request
    uint8_t request_count;
    uint16_t request_timestamp;
    uint32_t challenge;
    // Ping
    uint16_t ping_timestamp;
    long last_recv_ping;
    long last_sent_ping;
    // Keepalive
    uint8_t keepalive_sent_count;
    uint8_t keepalive_step;
    uint16_t keepalive_driver;
    uint32_t keepalive_timestamp;
    uint32_t keepalive_salt;
    long last_recv_keepalive;
    // Interface for controller
    void set_auth(struct drcom_auth *auth_info);
    void reset();
    void close();
    
private:
    Config *config;
    // Login
    uint8_t checksum[16];
    struct drcom_auth auth_info;
    
};
#endif
