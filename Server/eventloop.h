#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include "channel.h"
#include "poller.h"
#include <iostream>
namespace myserver {

class EventLoop {
public:
    using Functor = std::function<void()>;
    using ChannelList = std::vector<Channel*>;

    EventLoop();
    ~EventLoop();

    void loop();

    void quit() { quit_ = true; }
    std::thread::id getThreadId() {return tid_;}

    void addChannel(Channel *ch) {
        poller_.addChannel(ch);
    }

    void updateChannel(Channel *ch) {
        poller_.updateChannel(ch);
    }

    void removeChannel(Channel *ch) {
        poller_.removeChannel(ch);
    }

    // worker thread
    // 唤醒后的处理
    void wakeUp();
    void handleRead();
    void handleError();
    void addTask(Functor func);
    void executeTask();
private:
    bool quit_;
    std::thread::id tid_;

    Poller poller_;

    // 活跃事件
    ChannelList activeChannelList_;

    // 任务列表
    std::vector<Functor> functionList_;
    std::mutex mutex_;
    // 跨线程唤醒
    int wakeupfd_; //用于监听唤醒事件
    Channel wakeupchannel_;
};
}
#endif // EVENTLOOP_H
