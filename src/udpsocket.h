#ifndef UDPSOCKET_H
#define UDPSOCKET_H
#include "mysocket.h"

class UdpSocket : public MySocket
{
public:
    UdpSocket() = default;
    ~UdpSocket() = default;
    int init(string, int, int);
    int sendData(uint8_t*, int);
};

#endif //UDPSOCKET_H