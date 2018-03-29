#include "receiver.h"
#include "common/log.h"
#include "common/drcom.h"
#include <memory.h>

Receiver::Receiver(Controller *controller) {
    this->controller = controller;
    this->enabled = true;
}

void Receiver::loop() {
    unsigned char buf[500], pkt_type;
    ssize_t recv_len = 0;
    while (true) {
        if (!enabled)
            return;
        memset(buf, 0, sizeof(buf));
        if ((recv_len = controller->receive(buf, sizeof(buf))) == SOCKET_ERROR)
            continue;
        pkt_type = buf[0];
        if (!controller->is_message(buf, recv_len)) {
            if (controller->verify(buf, recv_len))
                controller->complete_step();
            else
                continue;
        }
        switch (pkt_type) {
            case PKT_CHALLENGE:
                controller->on_challenge((struct drcom_challenge *)buf);
                break;
            case PKT_ACK_SUCCESS:
                controller->on_auth_success((struct drcom_acknowledgement *)buf);
                break;
            case PKT_ACK_FAILURE:
                controller->on_auth_fail((struct drcom_ack_header *)buf);
                break;
            case PKT_KEEPALIVE:
                if (controller->is_ping_ack(buf, recv_len))
                    controller->on_ping((struct drcom_ping_ack *)buf);
                else
                    controller->on_keepalive((struct drcom_keepalive *)buf);
                break;
            case PKT_MESSAGE:
                controller->on_message((struct drcom_message *)buf, recv_len);
                break;
            default:
                break;
        }
    }
}
