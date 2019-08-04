#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>

namespace myserver {
class Channel{
public:
    using Callback = std::function<void()>;

    Channel() : fd_(-1) {}

    void setFd(int fd) {fd_ = fd;}
    int getFd() const {return fd_;}

    void setEvents(uint32_t events) { events_ = events; }
    uint32_t getEvents() const {return events_; }

    void handleEvent();

    void setReadHandler(const Callback &cb) {readhandler_ = cb;}
    void setWriteHandler(const Callback &cb) {writehandler_ = cb;}
    void setErrorHandler(const Callback &cb) {errorhandler_ = cb;}
    void setCloseHandler(const Callback &cb) {closehandler_ = cb;}
private:
    int fd_;
    uint32_t events_;

    Callback readhandler_;
    Callback writehandler_;
    Callback errorhandler_;
    Callback closehandler_;
};
}

#endif // CHANNEL_H
