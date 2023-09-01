#ifndef __THREADPOOL__
#define __THREADPOOL__

#include "TaskQueue.h"
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>

using std::cout; using std::endl; using std::to_string;

class ThreadPool
{
public:
    ThreadPool(int min, int max);
    ~ThreadPool();

    // 添加任务
    void addTask(Task task);
    // 获取忙线程的个数
    int getBusyNumber();
    // 获取活着的线程个数
    int getAliveNumber();

private:
    // 工作的线程的任务函数
    static void* worker(void *arg);
    // 管理者线程的任务函数
    static void* manager(void *arg);
    // 单个线程退出
    void threadExit();

private:
    pthread_mutex_t m_lock;     //锁整个的线程池
    pthread_cond_t m_notEmpty;  //任务队列是否为空
    pthread_t *m_threadIDs;     //工作线程ID
    pthread_t m_managerID;      //管理线程ID
    TaskQueue *m_taskQ;         //任务队列
    int m_minNum;               //最小线程数量
    int m_maxNum;               //最大线程数量
    int m_busyNum;              //忙线程数量
    int m_aliveNum;             //存活线程数量
    int m_exitNum;              //需要销毁线程数量
    bool m_shutdown = false;    //是否销毁线程
};

#endif