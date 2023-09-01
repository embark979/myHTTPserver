#ifndef __ACCEPTOR__
#define __ACCEPTOR__

#include "InetAddress.h"
#include "Connection.h"
#include "Socket.h"
#include "utils.h"
#include "Timer.h"

#include <functional>

class Acceptor {
public:
    Acceptor( int _epollFd, Timer* _timer );
    ~Acceptor();
    void acceptConnection();
    Socket* getserverSocket() const { return serverSocket; }
    void setCallback( std::function<void()> _cb ) { Callback = _cb; }
    void handleFunction() { Callback(); }
    std::function<void()> Callback;
    std::unordered_map< int, Connection* > clientConnectionMap;

private:
    int epollFd;
    Socket* serverSocket;
    Timer* timer;
};

#endif