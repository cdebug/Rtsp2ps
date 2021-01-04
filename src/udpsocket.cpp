#include "udpsocket.h"

int UdpSocket::init(std::string serverIp, int serverPort, int localPort)
{
    if((m_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        std::cout << "Cannot init socket" << std::endl;
        return -1;
    }
    struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(localPort);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		std::cout << "bind error." << localPort << std::endl;
		return -1;
	}
    m_serverIp = serverIp;
    m_serverPort = serverPort;
    m_localPort = localPort;
    return 0;
}

int UdpSocket::sendData(uint8_t* data, int len)
{
    if(m_serverIp.empty() || m_serverPort < 0 || m_localPort < 0)
    {
        std::cout << m_serverIp << " " << m_serverPort << " " << m_localPort << std::endl;
        return -1;
    }
    struct sockaddr_in addr_serv;  
    int socklen;  
    memset(&addr_serv, 0, sizeof(addr_serv));  
    addr_serv.sin_family = AF_INET; 
    addr_serv.sin_addr.s_addr = inet_addr(m_serverIp.c_str());  
    addr_serv.sin_port = htons(m_serverPort);  
    socklen = sizeof(addr_serv);  
    return sendto(m_sockfd, data, len, 0, (struct sockaddr*)&addr_serv, socklen);
}