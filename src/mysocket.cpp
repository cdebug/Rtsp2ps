#include "mysocket.h"

string MySocket::serverIp()
{
    return m_serverIp;
}

int MySocket::serverPort()
{
    return m_serverPort;
}

int MySocket::localPort()
{
    return m_localPort;
}