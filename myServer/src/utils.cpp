#include "utils.h"

char *memory_index_page = nullptr; // 后面放在CPP中去

void addFdToEpoll_INLT( int epollfd, int addedfd ) {
    struct epoll_event epollEvent;
    memset( &epollEvent, 0, sizeof( epollEvent ) );
    epollEvent.data.fd = addedfd;
    epollEvent.events = ( EPOLLIN ); // 可读事件 + 电平触发模式
    epoll_ctl( epollfd, EPOLL_CTL_ADD, addedfd, &epollEvent); // socketfd在epoll_event需要设置在epoll_ctl()函数中也需要设置？
}


void addFdToEpoll_INET( int epollfd, int addedfd ) {
    struct epoll_event epollEvent;
    memset( &epollEvent, 0, sizeof( epollEvent ) );
    epollEvent.data.fd = addedfd;
    epollEvent.events = ( EPOLLIN | EPOLLET ); // 可读事件 + 边缘触发模式
    epoll_ctl( epollfd, EPOLL_CTL_ADD, addedfd, &epollEvent); // socketfd在epoll_event需要设置在epoll_ctl()函数中也需要设置？
}

void addFdToEpoll_INETONESHOT( int epollfd, int addedfd ) {
    struct epoll_event epollEvent;
    memset( &epollEvent, 0, sizeof( epollEvent ) );
    epollEvent.data.fd = addedfd;
    epollEvent.events = ( EPOLLIN | EPOLLET | EPOLLONESHOT ); // 可读事件 + 边缘单次触发模式
    epoll_ctl( epollfd, EPOLL_CTL_ADD, addedfd, &epollEvent); // socketfd在epoll_event需要设置在epoll_ctl()函数中也需要设置？
}

void resetOneshot_INETONESHOT( int epollfd, int resetfd ) {
    struct epoll_event epollEvent;
    memset( &epollEvent, 0, sizeof( epollEvent ) );
    epollEvent.data.fd = resetfd;
    epollEvent.events = ( EPOLLIN | EPOLLET | EPOLLONESHOT ); // 可读事件 + 边缘单次触发模式
    epoll_ctl( epollfd, EPOLL_CTL_MOD, resetfd, &epollEvent);
}

void deleteFdFromEpoll( int epollfd, int targetfd ) {
    epoll_ctl( epollfd, EPOLL_CTL_DEL, targetfd, nullptr );
}

int setFdNonblock( int fd ) {
    int old_options = fcntl( fd, F_GETFL );
    int new_options = old_options | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_options );
    return old_options;
}

bool fileExist( const char path [] ) {
    struct stat staticFileState;
    int ret = stat( path, &staticFileState );
    if( ret == -1 ){  // 没有对应的文件和文件夹或拒绝存取
        return false; 
    }
    else return true;
}

void dispPeerConnection( int clientFd ) {
    struct sockaddr_in clientaddr;
    socklen_t clientaddrLength = sizeof( clientaddr );
    std::cout << " Client Fd: " << clientFd;
    getpeername( clientFd, reinterpret_cast< struct sockaddr* >(&clientaddr), &clientaddrLength);
    dispAddrInfo( clientaddr );
}

void dispAddrInfo( struct sockaddr_in& addr ) {
    std::cout << " , Client IP: " << inet_ntoa( addr.sin_addr ) << " , Client Port: " << ntohs( addr.sin_port ) << std::endl;
}
