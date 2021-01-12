#ifndef MYSOCKET_H
#define MYSOCKET_H
#include "common.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class MySocket
{
public:
    virtual int init(string, int, int) = 0;
    virtual int sendData(uint8_t*, int) = 0;
    string serverIp();
    int serverPort();
    int localPort();
protected:
    string m_serverIp;
    int m_serverPort = -1;
    int m_localPort = -1;
    int m_sockfd;
};

#endif //MYSOCKET_H