#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include "eventloop.h"
#include "eventloopthread.h"
#include <vector>

namespace myserver {

class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* maineventloop, const int eventloopnum = 0);
    ~EventLoopThreadPool();

    void startAllThread();

    EventLoop* getNextLoop();
private:
    EventLoop* maineventloop_;
    std::vector<EventLoopThread*> eventloopthreads_;
    int eventloopnum_;

    int index_;
};
}
#endif // EVENTLOOPTHREADPOOL_H
