#ifndef __SOCKET__
#define __SOCKET__

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <exception>
#include <iostream>

#include "InetAddress.h"

// 用于管理描述符

class Socket {
public:
    Socket();
    Socket(int);
    ~Socket();
    void bind( InetAddress* );
    void listen( int );
    void setnonblocking();
    void setAddrReuse();
    int accept( InetAddress* );
    int getSocketFd() const { return socketFd; }

private:
    int socketFd;
};

#endif