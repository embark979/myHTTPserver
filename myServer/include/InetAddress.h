#ifndef __INETADDRESS__
#define __INETADDRESS__

#include <arpa/inet.h>
#include <cstring>

// 主要用接收客户端信息

class InetAddress
{
public:
    struct sockaddr_in addr;
    socklen_t addr_len;
    InetAddress();
    InetAddress(const char* ip, uint16_t port);
    ~InetAddress();
};

#endif