#ifndef __TASKQUEUE__
#define __TASKQUEUE__

#include <queue>
#include <pthread.h>
#include <functional>

using callbackType = std::function<void()>; using std::queue;

struct Task {
    Task() { function = nullptr; }
    Task( callbackType f ) { function = f; }
    callbackType function;
};

class TaskQueue {
public:
    TaskQueue();
    ~TaskQueue();

    //添加任务
    void addTask( Task task );
    void addTask( callbackType f );

    //取任务
    Task takeTask();

    //内联函数不会进行压栈，而是进行代码块的替换
    inline int taskNumber() {return m_taskQ.size();}
    
private:
    queue<Task> m_taskQ;//存放任务
    pthread_mutex_t m_mutex;//互斥锁，防止多线程同时操作共享资源
};

#endif