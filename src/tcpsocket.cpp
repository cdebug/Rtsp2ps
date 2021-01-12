#include "tcpsocket.h"

int TcpSocket::init(string serverIp, int serverPort, int localPort)
{
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == m_sockfd) {
        return false;
    }

    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if( bind(m_sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        close(m_sockfd);
        return false;
    }

   	struct sockaddr_in addr_client;  
    /* 填写服务器地址: IP + 端口 */
    addr_client.sin_family = AF_INET;
    addr_client.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp.c_str(), &addr_client.sin_addr); /* 可通过返回值判断IP地址合法性 */

    /* client建链, TCP三次握手, 这里clientsockfd可以不显示绑定地址，内核会选client的本端IP并分配一个临时端口 */
    if (-1 == connect(m_sockfd, (const struct sockaddr *)&addr_client, sizeof(addr_client))) {
        close(m_sockfd);
        return false;
    }
    m_serverIp = serverIp;
    m_serverPort = serverPort;
    m_localPort = localPort;
    return 0;
}

int TcpSocket::sendData(uint8_t* data, int len)
{
    uint16_t dataLen = len;
    dataLen = htons(dataLen);
    send(m_sockfd, (uint8_t*)&dataLen, 2, 0);
    return send(m_sockfd, data, len, 0);
}