#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "socket.h"
#include "eventloop.h"
#include "channel.h"
#include <map>
#include <memory>
#include "tcpconnection.h"
#include <mutex>
#include "eventloopthreadpool.h"
#include <functional>
#include <string>

namespace myserver {

class TcpServer {
public:
    using SpTcpConnection = std::shared_ptr<TcpConnection>;
    using Callback = std::function<void(const SpTcpConnection&)>;
    using MessageCallback = std::function<void(const SpTcpConnection&, std::string&)>;

    TcpServer(EventLoop* eventloop, const int port, const int iothreadnum = 0);
    void start();

    void setMessageCallback(const MessageCallback& cb) {messagecallback_ = cb;}
    void setSendCompleteCallback(const Callback& cb) {sendcompletecallback_ = cb;}
    void setErrorCallback(const Callback& cb) {errorcallback_ = cb;}
    void setCloseCallback(const Callback& cb) {closecallback_ = cb;}
    void setNewConnectionCallback(const Callback& cb) {newconnectioncallback_ = cb;}
private:
    static const int MAXCONNECTION;

    EventLoop* eventloop_;
    Socket socket_;
    Channel channel_;

    int conncount_;

    std::map<int, std::shared_ptr<TcpConnection>> conntable;
    std::mutex mutex_; // used for conntable

    // IO thread pool
    EventLoopThreadPool eventloopthreadpool_;

    // user Callback
    Callback newconnectioncallback_;
    MessageCallback messagecallback_;
    Callback sendcompletecallback_;
    Callback closecallback_;
    Callback errorcallback_;

    void onNewConnection();
    void removeConnection(const std::shared_ptr<TcpConnection> spTcpConnection);
    void onConnectionError();
};
}
#endif // TCPSERVER_H
