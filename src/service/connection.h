#ifndef CONNECTION_H
#define CONNECTION_H

#define SOCKET_ERROR -1
#include <netinet/in.h>

class Connection {
private:
    int client;
    sockaddr_in serv_addr, client_addr;
    
public:
    Connection(uint32_t client_ip, uint16_t client_port, uint32_t serv_ip, uint16_t serv_port);
    ssize_t send(char *data, size_t len);
    ssize_t receive(unsigned char *buf, size_t len);
    void _close();
};
#endif
