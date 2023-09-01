#include "Socket.h"

Socket::Socket() : socketFd(-1) {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) throw std::runtime_error("socket create failed\n");
}

Socket::Socket(int _socketFd) : socketFd(_socketFd) {}

Socket::~Socket() {
    if (socketFd != -1) {
        close(socketFd);
        socketFd = -1;
    }
}

void Socket::bind( InetAddress* addr ) {
    int ret = ::bind(socketFd, reinterpret_cast<struct sockaddr*>(&addr->addr), addr->addr_len);
    if (ret < 0) throw std::runtime_error("bind serveraddr failed\n");
}

void Socket::listen( int maxNum ) {// maxNum: 允许最大排队的链接请求
    int ret = ::listen(socketFd, maxNum);
    if (ret < 0) throw std::runtime_error("call listen faile\n");
}

void Socket::setnonblocking() { fcntl(socketFd, F_SETFL, (fcntl(socketFd, F_GETFL) | O_NONBLOCK)); }

void Socket::setAddrReuse() {
    int opt = 1;
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

int Socket::accept( InetAddress* addr ) {
    int clientFd = ::accept(socketFd, reinterpret_cast<struct sockaddr*>(&addr->addr), &addr->addr_len);
    if (clientFd < 0) throw std::runtime_error("accept client faile\n");
    return clientFd;
}