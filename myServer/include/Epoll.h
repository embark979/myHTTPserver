#ifndef __EPOLL__
#define __EPOLL__

#include <sys/epoll.h>
#include <exception>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <functional>

#include "Timer.h"
#include "utils.h"
#include "Acceptor.h"
#include "ThreadPool.h"

#define MAX_EVENTS 1000
const int TIMEOUT = 15;

class Epoll {
public:
    Epoll();
    ~Epoll();
    void loop( int pipeline[2] );
    int getEpollFd() const { return epollFd; }
    
private:
    bool quit;
    int epollFd;
    Timer* timer;
    char signals[ 128 ];
    ThreadPool* threadPool;
};

#endif