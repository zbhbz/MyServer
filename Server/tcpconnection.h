#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "channel.h"
#include "eventloop.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <memory>
#include <string>
#include <functional>

namespace myserver {

class TcpConnection : public std::enable_shared_from_this<TcpConnection>{
public:
    using SpTcpConnection = std::shared_ptr<TcpConnection>;
    using Callback = std::function<void(const SpTcpConnection&)>;
    using MessageCallback = std::function<void(const SpTcpConnection&, std::string&)>;

    TcpConnection(EventLoop*, int fd, const struct sockaddr_in&);
    ~TcpConnection();

    int getFd() const {return fd_;}
    EventLoop* getEventLoop() const {return eventloop_;}

    void addChannelToEventLoop();
    void send(const std::string&);

    void sendInLoop();

    void shutdown();
    void shutdownInLoop();

    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();

    void setMessageCallback(const MessageCallback& cb) {messagecallback_ = cb;}
    void setSendCompleteCallback(const Callback& cb) {sendcompletecallback_ = cb;}
    void setErrorCallback(const Callback& cb) {errorcallback_ = cb;}
    void setCloseCallback(const Callback& cb) {closecallback_ = cb;}
    void setConnectCleanUpCallback(const Callback& cb) {connectcleanupcallback_ = cb;}

    void setAsyncProcessing(const bool flag) {asyncprocessing_ = flag;}
private:
    EventLoop* eventloop_;
    int fd_;
    std::unique_ptr<Channel> channel_;
    struct sockaddr_in clientaddr_;

    // buffer
    std::string bufferin_;
    std::string bufferout_;

    // flag
    bool halfclose_;
    bool disconnected_;
    bool asyncprocessing_;

    // callback func
    MessageCallback messagecallback_; // read complete
    Callback sendcompletecallback_;
    Callback errorcallback_;
    Callback closecallback_;
    Callback connectcleanupcallback_;
};
}
#endif // TCPCONNECTION_H
