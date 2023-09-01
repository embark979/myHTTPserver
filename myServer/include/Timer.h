#ifndef __TIMER__
#define __TIMER__

#include <sys/time.h>
#include <unordered_map>
#include <iostream>
#include <list>
#include <pthread.h>

#include "unistd.h"
#include "utils.h"

class Timer {
public:
    Timer( int _epollfd, int _timeOut, int _maxSize = 1000 ) : epollFd( _epollfd ), timeOut( _timeOut ),  maxSize( _maxSize ), curSize( 0 ) {
        if ( pthread_mutex_init( &m_lock, nullptr ) != 0 ) throw std::runtime_error("mutex init error!\n");
    }
    bool addFd( int clientFd );
    void deleteExpiredFd();
    void deleteFd( int clientFd );
    void forbidenFd( int clientFd );
private:
    class Node{
    public:
        using TimeVal = struct timeval;
        // _timeout表示长连接的超时时间，默认10s
        Node( int _clientfd, int _timeOut );

        const TimeVal& curTime() const { return m_curTime; }
        const TimeVal& expireTime() const { return m_expireTime; }
        const int clientFd() const { return m_clientFd; }
        void resetExpireTime( int _timeOut );
        void setExpireTime( int _timeOut );
    private:
        int m_clientFd;
        TimeVal m_curTime; // 用于获取当前时间
        TimeVal m_expireTime;
    };

    pthread_mutex_t m_lock;     //锁整个的线程池
    std::list< Node > timeList; // 保存节点的链表，快失效的在链表头，新的在链表尾部
    std::unordered_map< int, std::list< Node >::iterator > hashMap; // 保存客户端套接字文件描述符到时间节点的映射关系
    
    int epollFd;
    int timeOut;
    int maxSize;
    int curSize;

};

#endif