
#include "eventloop.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/eventfd.h>

int createEventfd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0) {
        perror("create eventfd fail");
        exit(-1);
    }
    return fd;
}

myserver::EventLoop::EventLoop() : quit_(true),
    tid_(std::this_thread::get_id()),
    wakeupfd_(createEventfd()) {
    wakeupchannel_.setFd(wakeupfd_);
    wakeupchannel_.setEvents(EPOLLIN | EPOLLET);
    wakeupchannel_.setReadHandler(std::bind(&EventLoop::handleRead, this));
    wakeupchannel_.setErrorHandler(std::bind(&EventLoop::handleError, this));
    addChannel(&wakeupchannel_);
}

myserver::EventLoop::~EventLoop() {
    ::close(wakeupfd_);
}

void myserver::EventLoop::addTask(Functor func) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functionList_.push_back(func);
    }
    wakeUp(); //跨线程唤醒，worker线程唤醒IO线程
}

// run in work thread
void myserver::EventLoop::executeTask() {
    std::vector<Functor> functionListCopy;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functionListCopy.swap(functionList_);
    }
    for (auto &func : functionListCopy) {
        func();
    }
}

void myserver::EventLoop::wakeUp() {
    uint64_t one = 1;
    size_t n = write(wakeupfd_, (char*)&one, sizeof(one));
}

void myserver::EventLoop::handleRead() {
    //std::cout << "handleRead EventLoop fd:" << wakeupfd_ << std::endl;
    uint64_t one = 1;
    size_t n = read(wakeupfd_, &one, sizeof(one));
}

void myserver::EventLoop::handleError() {}

void myserver::EventLoop::loop() {
    quit_ = false;
    while (!quit_) {
        poller_.poll(activeChannelList_);

        for (auto channel : activeChannelList_) {
            channel->handleEvent();
        }
        activeChannelList_.clear();
        executeTask();
    }
}
