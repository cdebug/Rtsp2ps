#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include "mysocket.h"

class TcpSocket : public MySocket
{
public:
    TcpSocket() = default;
    ~TcpSocket() = default;
    int init(std::string, int, int);
    int sendData(uint8_t*, int);
};

#endif //TCPSOCKET_H