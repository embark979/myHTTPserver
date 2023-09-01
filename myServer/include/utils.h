#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <signal.h>
#include <iostream>

extern char *memory_index_page;

class SocketClosed : public std::exception {
public:
    SocketClosed() {}
    virtual const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW{
        return "socket closed";
    }
    ~SocketClosed() throw() {}
};

// 向epoll中添加需要监听的文件描述符;可读事件配合电平触发
void addFdToEpoll_INLT( int epollfd, int addedfd );
// 向epoll中添加需要监听的文件描述符;可读事件配合边缘触发
void addFdToEpoll_INET( int epollfd, int addedfd );
// 向epoll中添加需要监听的文件描述符;可读事件配合边缘单次触发，防止在epoll中，正在被其他线程处理的客户端链接被探测到，导致客户端链接被重复传入线程池
void addFdToEpoll_INETONESHOT( int epollfd, int addedfd );
// 重置Epoll的oneshot设置
void resetOneshot_INETONESHOT( int epollfd, int resetfd );
// 从epoll中删除被监听的文件描述符
void deleteFdFromEpoll( int epollfd, int targetfd );
// 设置文件描述符为非阻塞
int setFdNonblock( int fd );


bool fileExist( const char path [] );
void dispPeerConnection( int clientFd );
void dispAddrInfo( struct sockaddr_in& addr );

#endif