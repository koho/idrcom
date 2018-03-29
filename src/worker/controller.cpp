#include "controller.h"
#include "common/log.h"
#include <string.h>

Action Controller::actions[5] = {&Service::send_request, &Service::send_login, &Service::send_logout, &Service::send_ping, &Service::send_keepalive};

Controller::Controller(Service *service) {
    this->service = service;
}

void Controller::bind(Receiver *receiver, Sender *sender) {
    this->receiver = receiver;
    this->sender = sender;
}

ssize_t Controller::receive(unsigned char *buf, size_t len) {
    return service->connection.receive(buf, len);
}

void Controller::on_challenge(struct drcom_challenge *challenge_packet) {
    service->challenge = challenge_packet->challenge;
    if (service->step == STEP_REQ_LOGIN) {
        service->step = STEP_SEND_LOGIN;
        sender->execute(TASK_SEND_LOGIN);
    }
    else if (service->step == STEP_REQ_LOGOUT) {
        service->step = STEP_SEND_LOGOUT;
        sender->execute(TASK_SEND_LOGOUT);
    }
}

void Controller::on_auth_success(struct drcom_acknowledgement *ack) {
    if (service->step == STEP_SEND_LOGIN){
        Log::i("auth: sucess, code %d", ack->ack_header.error_code);
        service->logged_in = true;
        service->set_auth(&ack->auth_info);
        service->step = STEP_KEEPALIVE_1_2;
        sender->execute(TASK_SEND_KEEPALIVE);
    }
    else if (service->step == STEP_SEND_LOGOUT) {
        // Stop auth
        Log::i("auth: sucess, code %d", ack->ack_header.error_code);
        try_exit();
    }
}

void Controller::on_auth_fail(struct drcom_ack_header *ack_header) {
    const char *error_desc = "unknown error";
    switch (ack_header->error_code) {
        case ERROR_USERNAME_PASSWD_1:
        case ERROR_USERNAME_PASSWD_2:
            error_desc = "wrong username or password";
            break;
        case ERROR_ACCOUNT_DISABLE:
            error_desc = "your account is disabled";
            break;
        case ERROR_TIME_TRAFFIC_OVERFLOW:
            error_desc = "time or data limit reached";
            break;
        case ERROR_IP_STATIC:
            error_desc = "static IP is used";
            break;
        default:
            break;
    }
    Log::e("auth: fail, code %d, %s", ack_header->error_code, error_desc);
    // Stop auth
    try_exit();
}

bool Controller::is_message(unsigned char *packet, ssize_t len) {
    return packet[0] == PKT_MESSAGE || is_ping_ack(packet, len);
}

bool Controller::is_ping_ack(unsigned char *packet, ssize_t len) {
    if (len != sizeof(struct drcom_ping_ack))
        return false;
    struct drcom_ping_ack *ping_ack = (struct drcom_ping_ack *)packet;
    if (ping_ack->header.pkt_type == PKT_PING_ACK &&
        ping_ack->header.len == sizeof(struct drcom_ping_ack_header) &&
        ping_ack->header.flag == 6)
        return true;
    return false;
}

void Controller::on_ping(struct drcom_ping_ack *ping) {
    // It's ok to not verify driver
    if (ping->header.timestamp == service->ping_timestamp) {
        // Ping ACK received
        service->last_recv_ping = time(NULL);
    }
}

void Controller::on_keepalive(struct drcom_keepalive *keepalive) {
    service->keepalive_sent_count++;
    service->last_recv_keepalive = time(NULL);
    switch (service->step) {
        case STEP_KEEPALIVE_1_2:
            if (keepalive->step == 6) {
                service->keepalive_driver = keepalive->driver;
            } else {
                service->keepalive_step = keepalive->step + 1;
                service->keepalive_salt = keepalive->salt1;
                service->step = STEP_KEEPALIVE_3_4;
            }
            sender->execute(TASK_SEND_KEEPALIVE);
            break;
        case STEP_KEEPALIVE_3_4:
            service->keepalive_step = 1;
            service->step = STEP_KEEPALIVE_1_2;
            sender->execute(TASK_SEND_KEEPALIVE, 18);
            break;
        default:
            break;
    }
}

void Controller::on_message(struct drcom_message *message, ssize_t len) {
    switch (message->flag) {
        case ERROR_ADMIN_RESET:
            if (len < 14)
                return;
            if (memcmp(message->command, "AdminReset", sizeof(message->command)) == 0) {
                char *ip;
                if (len > 14 && (ip = strstr(message->detail, ">")) != 0)
                    Log::i("msg: this account has logged in from other places (%s)", ip + 1);
                else
                    Log::i("msg: this login was forced off by the admin");
                service->reset();
                sender->execute(TASK_SEND_REQUEST);
            }
            break;
        case 0x26:
            /* Not handled in this client */
            break;
        default:
            break;
    }
}

bool Controller::verify(unsigned char *packet, ssize_t len) {
    switch (this->service->step) {
        case STEP_REQ_LOGIN:
        case STEP_REQ_LOGOUT:
            if (len != sizeof(struct drcom_challenge))
                return false;
        {
            struct drcom_challenge *challenge_packet = (struct drcom_challenge *)packet;
            if (challenge_packet->serv_header.pkt_type == PKT_CHALLENGE &&
                challenge_packet->serv_header.seq == service->request_count &&
                challenge_packet->serv_header.timestamp == service->request_timestamp)
                return true;
        }
            break;
        case STEP_SEND_LOGIN:
        case STEP_SEND_LOGOUT:
            if (len < sizeof(struct drcom_ack_header))
                return false;
            if (packet[0] == PKT_ACK_SUCCESS || packet[0] == PKT_ACK_FAILURE)
                return true;
            break;
        case STEP_KEEPALIVE_1_2:
        case STEP_KEEPALIVE_3_4:
            if (len < sizeof(struct drcom_keepalive))
                return false;
        {
            struct drcom_keepalive *keepalive_packet = (struct drcom_keepalive *)packet;
            if (keepalive_packet->pkt_type == PKT_KEEPALIVE &&
                keepalive_packet->seq == service->keepalive_sent_count &&
                keepalive_packet->len == len &&
                keepalive_packet->flag == 0x0b &&
                keepalive_packet->timestamp == service->keepalive_timestamp) {
                if (keepalive_packet->step == service->keepalive_step + 1)
                    return keepalive_packet->driver == service->keepalive_driver;
                else if (keepalive_packet->step == 6 && service->step == STEP_KEEPALIVE_1_2)
                    return true;
            }
        }
            break;
        default:
            break;
    }
    return false;
}

void Controller::complete_step() {
    this->sender->notify();
}

ssize_t Controller::send_task(TaskType type) {
    return (service->*actions[type])();
}

void Controller::try_exit() {
    service->logged_in = false;
    receiver->enabled = false;
    service->close();
}

void Controller::on_wait_timedout(unsigned int wait, int try_count) {
    Log::w("connection timed out: %s at %ds, try %d", step_desc[service->step], wait, try_count);
}

void Controller::on_interrupt() {
    if (service->logged_in) {
        service->step = STEP_REQ_LOGOUT;
        sender->execute(TASK_SEND_REQUEST, 0, 2, 2, true);
    } else {
        try_exit();
    }
}

bool Controller::should_ping() {
    if (!service->logged_in)
        return false;
    if (service->step == STEP_REQ_LOGOUT)
        return true;
    // We have never sent ping
    // so we send immediately
    if (service->last_sent_ping == 0)
        return true;
    if (time(NULL) - service->last_sent_ping >= 10)
        return true;
    return false;
}

bool Controller::should_exit() {
    return service->step >= STEP_REQ_LOGOUT;
}

void Controller::restart() {
    service->reset();
}
