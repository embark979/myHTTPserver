#include "Acceptor.h"
#include "Connection.h"
#include "HttpData.h"
#include "InetAddress.h"
#include "Pages.h"
#include "Responser.h"
#include "Socket.h"
#include "Timer.h"
#include "utils.h"
#include "Epoll.h"

#include <signal.h>

static int pipeline[2];// 统一信号源的管道

// 信号处理句柄
void signalHandler( int sig ) {
    int saveErrno = errno;
    int msg = sig;
    write( pipeline[ 1 ], reinterpret_cast<char*>( &msg ), 1 ); // fixme 
    errno = saveErrno;
}

// 设置句柄
void addSingal( int sig ) {
    struct sigaction sa;
    memset( &sa, 0, sizeof( sa ) );
    sa.sa_handler = signalHandler;
    sa.sa_flags |= SA_RESTART; // ??????这个是做什么的
    sigfillset( &sa.sa_mask ); // 利用sigfillset函数初始化sa_mask，设置所有的信号
    sigaction( sig, &sa, nullptr );
}

int main() {
    Epoll* epollTree = new Epoll();

    int ret = pipe( pipeline );
    if( ret == -1 ) throw std::runtime_error("pipeline create failed\n");

    addFdToEpoll_INLT( epollTree->getEpollFd(), pipeline[ 0 ] );
    addSingal( SIGALRM );
    setFdNonblock( pipeline[ 1 ] );
    alarm( 1 );

    epollTree->loop( pipeline );

    return 0;
}