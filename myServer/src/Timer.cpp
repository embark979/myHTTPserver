#include "Timer.h"

Timer::Node::Node(int _clientFd, int _timeout ) : m_clientFd( _clientFd ) {
    setExpireTime( _timeout );
}

void Timer::Node::resetExpireTime( int _timeout ) {
    setExpireTime( _timeout );
}

void Timer::Node::setExpireTime( int _timeout ) {
    gettimeofday( &m_curTime, nullptr );
    m_expireTime = m_curTime;
    m_expireTime.tv_sec += _timeout;
}

bool Timer::addFd( int clientFd ) {
    pthread_mutex_lock( &m_lock );

    if ( curSize >= maxSize ) return false;
    if ( hashMap.find( clientFd ) == hashMap.end() ) { // 当前文件描述符不在队列中
        hashMap[ clientFd ] = timeList.insert( timeList.end(), Node( clientFd, timeOut ) );
        ++curSize;

        std::cout << "Timer - add client to keep alive:";
        dispPeerConnection( clientFd );
        addFdToEpoll_INETONESHOT( epollFd, clientFd ); // 同时上树
        std::cout << "Client " << clientFd << " add to EpollTree" << std::endl;
    }
    else { // 文件描述符未过期, 将其从链表中删除, 并且插入到链表的最后
        hashMap[ clientFd ] = timeList.insert( timeList.end(), Node( clientFd, timeOut ) );
        resetOneshot_INETONESHOT( epollFd, clientFd ); // 此时已经在树中的, 所以进行MOD
    }

    pthread_mutex_unlock( &m_lock );
    return true;
}

void Timer::deleteExpiredFd() {
    pthread_mutex_lock( &m_lock );

    Node::TimeVal curTime;
    gettimeofday( &curTime, nullptr );
    while ( curSize != 0 && curTime.tv_sec > timeList.front().expireTime().tv_sec ) {
        hashMap.erase( timeList.front().clientFd() );
        std::cout << "Timer - delete expire keep alive:";
        dispPeerConnection( timeList.front().clientFd() );

        deleteFdFromEpoll( epollFd, timeList.front().clientFd() );
        std::cout << "Client " << timeList.front().clientFd() << " delete from EpollTree" << std::endl;

        close( timeList.front().clientFd() );
        timeList.pop_front();
        --curSize;
    }

    pthread_mutex_unlock( &m_lock );
}

void Timer::deleteFd( int clientFd ) {
    pthread_mutex_lock( &m_lock );
    
    if ( hashMap.find( clientFd ) == hashMap.end() ) {}
    else {
        timeList.erase( hashMap[ clientFd ] );
        hashMap.erase( clientFd );
        --curSize;
    }

    pthread_mutex_unlock( &m_lock );
}

void Timer::forbidenFd( int clientFd ) {
    pthread_mutex_lock( &m_lock );

    if ( hashMap.find( clientFd ) != hashMap.end() ) { // 该fd存在
        timeList.erase( hashMap[ clientFd ] );
        hashMap[ clientFd ] = timeList.end();
    }

    pthread_mutex_unlock( &m_lock );
}