#include "Epoll.h"

Epoll::Epoll() : epollFd( -1 ), quit( false ) { // 创建epollFd, 以及Timer和ThreadPool
    epollFd = epoll_create( 128 );
    if ( epollFd == -1 ) throw std::runtime_error("epoll create failed\n");

    timer = new Timer( getEpollFd(), TIMEOUT );
    threadPool = new ThreadPool( 3, 10 );
}

Epoll::~Epoll() {
    if ( epollFd != -1 ) { close( epollFd ); epollFd = -1; }
    delete timer;
    delete threadPool;
}

void Epoll::loop( int pipeline[2] ) {
    Acceptor* serverAcceptor = new Acceptor( getEpollFd(), timer ); // 创建ServerListeningFd, 以及相应的操作

    struct epoll_event epollEvents[128];

    while( !quit ) {
        int ret = epoll_wait( epollFd, epollEvents, 128, -1 );
        for ( int i = 0; i < ret; ++i ) {

            if ( epollEvents[i].data.fd == serverAcceptor->getserverSocket()->getSocketFd() ) { // 如果是监听描述符, 直接打包成任务丢进线程池
                std::function<void()> callback = std::bind( &Acceptor::handleFunction, serverAcceptor );
                threadPool->addTask( Task( callback ) );
            }

            else if ( epollEvents[i].data.fd != pipeline[ 0 ] ) {
                timer->forbidenFd( epollEvents[i].data.fd );
                auto clientConnection = serverAcceptor->clientConnectionMap[ epollEvents[i].data.fd ];
                std::function<void()> callback = std::bind( &Connection::handleFunction, clientConnection );
                threadPool->addTask( Task( callback ) );
            }

            else {
                int sigNums = read( pipeline[ 0 ], signals, 128 );
                for ( int i = 0; i < sigNums; ++i ) {
                    if ( signals[ i ] == SIGALRM ) {
                        timer->deleteExpiredFd();
                        alarm( 1 );
                    }
                    else continue;
                }
            }
        }
    }
}
