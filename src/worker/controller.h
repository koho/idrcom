#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "receiver.h"
#include "sender.h"
#include "service/service.h"

typedef ssize_t(Service::*Action)();

class Sender;
class Receiver;

class Controller {
private:
    Service *service;
    Receiver *receiver;
    Sender *sender;
    static Action actions[5];
    
public:
    Controller(Service *service);
    void bind(Receiver *receiver, Sender *sender);
    // Interface for receiver
    ssize_t receive(unsigned char *buf, size_t len);
    bool is_message(unsigned char *packet, ssize_t len);
    bool is_ping_ack(unsigned char *packet, ssize_t len);
    bool verify(unsigned char *packet, ssize_t len);
    void complete_step();
    void on_challenge(struct drcom_challenge *challenge_packet);
    void on_auth_success(struct drcom_acknowledgement *ack);
    void on_auth_fail(struct drcom_ack_header *ack_header);
    void on_keepalive(struct drcom_keepalive *keepalive);
    void on_ping(struct drcom_ping_ack *ping);
    void on_message(struct drcom_message *message, ssize_t len);
    // Interface for sender
    ssize_t send_task(TaskType type);
    void on_wait_timedout(unsigned int wait, int try_count);
    void try_exit();
    void restart();
    bool should_ping();
    bool should_exit();
    // Interface for user
    void on_interrupt();
};

#endif
