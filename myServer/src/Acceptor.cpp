#include "Acceptor.h"

Acceptor::Acceptor( int _epollFd, Timer* _timer ) : epollFd( _epollFd ), timer( _timer ) {
    serverSocket = new Socket();
    InetAddress* serverAddr = new InetAddress("127.0.0.1", 20979);
    serverSocket->setAddrReuse();
    serverSocket->bind(serverAddr);
    serverSocket->listen(128); 
    serverSocket->setnonblocking();

    addFdToEpoll_INET( epollFd, serverSocket->getSocketFd() ); // listenFd采用ET触发

    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    setCallback( cb );

    delete serverAddr;
}

Acceptor::~Acceptor() {
    delete serverSocket; // close的操作在Socket里面完成
}

void Acceptor::acceptConnection() {
    InetAddress* clientAddr = new InetAddress();
    Socket* clientSocket = new Socket( serverSocket->accept(clientAddr) );
    Connection* newConnectionClient = new Connection( clientSocket, timer );

    std::cout << "Accept client: SocketFD --> " << clientSocket->getSocketFd() << ", Port --> " << ntohs(clientAddr->addr.sin_port) << std::endl; 

    newConnectionClient->httpServerEvent(); // 链接的Socket就有数据发来, 先不上树, 直接处理, 如果是长连接再进行上树

    if ( newConnectionClient->isKeepConnection ) {
        clientConnectionMap[ clientSocket->getSocketFd() ] = newConnectionClient;
    } else delete newConnectionClient;

    delete clientAddr;
}