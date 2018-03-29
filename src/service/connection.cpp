#include "connection.h"
#include "common/log.h"
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>

Connection::Connection(uint32_t client_ip, uint16_t client_port, uint32_t serv_ip, uint16_t serv_port) {
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == SOCKET_ERROR) {
        Log::e("socket: %s", strerror(errno));
        exit(1);
    }
    memset(&client_addr, 0, sizeof(sockaddr_in));
    memset(&serv_addr, 0, sizeof(sockaddr_in));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = client_ip;
    client_addr.sin_port = htons(client_port);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = serv_ip;
    serv_addr.sin_port = htons(serv_port);
    if (bind(client, (sockaddr *)&client_addr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        Log::e("bind: %s", strerror(errno));
        exit(1);
    }
}

ssize_t Connection::send(char *data, size_t len) {
    return sendto(client, data, len, 0, (sockaddr *)&serv_addr, sizeof(sockaddr_in));
}

ssize_t Connection::receive(unsigned char *buf, size_t len) {
    sockaddr_in from_addr;
    socklen_t addr_len = sizeof(sockaddr_in);
    ssize_t bytes_read = recvfrom(client, buf, len, 0, (sockaddr *)&from_addr, &addr_len);
    return bytes_read <= 0 ? SOCKET_ERROR : (from_addr.sin_addr.s_addr == serv_addr.sin_addr.s_addr ? bytes_read : SOCKET_ERROR);
}

void Connection::_close() {
    close(client);
}
