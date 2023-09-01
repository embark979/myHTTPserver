#ifndef __CONNECTION__
#define __CONNECTION__

#include "Timer.h"
#include "Socket.h"
#include "HttpData.h"
#include "Responser.h"

#include <functional>

class Connection {
public:
    Connection( Socket* _clientSocket, Timer* _timer );
    ~Connection() ;
    void httpServerEvent();
    std::function<void()> Callback;
    void setCallback( std::function<void()> _cb ) { Callback = _cb; }
    void handleFunction() { Callback(); };
    bool isKeepConnection;

private:
    Socket* clientSocket;
    Timer* timer;
};

#endif