#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include "eventloop.h"
#include <thread>
#include <string>

namespace myserver {

class EventLoopThread {
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* getEventLoop();
    void startThread();
    void threadFunc();
private:
    std::string threadname_;
    std::thread::id tid_;
    std::thread thread_;
    EventLoop* eventloop_;
};
}
#endif // EVENTLOOPTHREAD_H
