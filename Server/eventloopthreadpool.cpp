#include "eventloopthreadpool.h"

myserver::EventLoopThreadPool::EventLoopThreadPool(EventLoop* maineventloop, const int eventloopnum) :
    maineventloop_(maineventloop), eventloopnum_(eventloopnum), index_(0) {
    for (int i = 0; i < eventloopnum_; ++ i) {
        eventloopthreads_.push_back(new EventLoopThread());
    }
}

myserver::EventLoopThreadPool::~EventLoopThreadPool() {
    for (int i = 0; i < eventloopnum_; ++ i) {
        delete eventloopthreads_[i];
    }
}

void myserver::EventLoopThreadPool::startAllThread() {
    for (int i = 0; i < eventloopnum_; ++ i) {
        eventloopthreads_[i]->startThread();
    }
}

myserver::EventLoop* myserver::EventLoopThreadPool::getNextLoop() {
    if (eventloopnum_ > 0) {
        EventLoop* temp = eventloopthreads_[index_]->getEventLoop();
        index_ = (index_ + 1) % eventloopnum_;
        return temp;
    }
    return maineventloop_;
}
