#include "Connection.h"

Connection::Connection( Socket* _clientSocket, Timer* _timer  ) : isKeepConnection( false ) {
    clientSocket = _clientSocket;
    timer = _timer;
    std::function<void()> cb = std::bind(&Connection::httpServerEvent, this);
    setCallback( cb );
}

Connection::~Connection() {
    delete clientSocket;
}

void Connection::httpServerEvent()
{
    HttpData httpData( clientSocket->getSocketFd() );
    httpData.parseData();

    if ( httpData.badRequest() )
    {
#ifdef __PRINT_INFO_TO_DISP__
        std::cout << "bad request" << std::endl;
#endif
        return;
    }
    if ( httpData.getRequestMethod() == HttpData::UNSUPPORT )
    {
#ifdef __PRINT_INFO_TO_DISP__
        std::cout << "unsupport method" << std::endl;
#endif
        return;
    }

    Responser responser( httpData );

    if ( !fileExist(httpData.getUrl().c_str() ) && httpData.getUrlResourceType() != "memory") {
        responser.sendNotFound();
        return;
    }

    if ( httpData.getRequestMethod() == HttpData::RequestMethod::GET ) {
        if (httpData.getUrlResourceType() == "memory") responser.sendMemoryPage();
        else responser.sendStaticFileToClient();
    }

    isKeepConnection = httpData.keepConnection();

    if ( httpData.keepConnection() ) {
        timer->addFd( clientSocket->getSocketFd() );
    }
    else {
        close( clientSocket->getSocketFd() );
    }
}