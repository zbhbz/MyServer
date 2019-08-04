#include "eventloopthread.h"
#include <iostream>
#include <sstream>

myserver::EventLoopThread::EventLoopThread() : threadname_("IO thread"),
    tid_(-1), eventloop_(nullptr){}

myserver::EventLoopThread::~EventLoopThread() {
    std::cout << "clean up the EventLoopThread in thread" << std::this_thread::get_id() << std::endl;
    eventloop_->quit();
    thread_.join();
}

myserver::EventLoop* myserver::EventLoopThread::getEventLoop() {
    return eventloop_;
}

void myserver::EventLoopThread::startThread() {
    thread_ = std::thread(&EventLoopThread::threadFunc, this);
}

void myserver::EventLoopThread::threadFunc() {
    EventLoop eventloop;
    eventloop_ = &eventloop;
    tid_ = std::this_thread::get_id();
    std::stringstream ss;
    ss << tid_;
    threadname_ += ss.str();
    std::cout << "IO thread" << threadname_ << std::endl;
    try {
        eventloop_->loop();
    } catch (std::exception e) {
        std::cerr << "bad in threadFunc " << e.what() << std::endl;
    }
}
