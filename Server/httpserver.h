#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "eventloop.h"
#include "tcpconnection.h"
#include "timer.h"
#include "tcpserver.h"
#include "threadpool.h"
#include "httpsession.h"
#include <memory>
#include <mutex>
#include <map>
namespace myserver {

class HttpServer {
public:
    using SpTcpConnection = std::shared_ptr<TcpConnection>;
    using SpHttpSession = std::shared_ptr<HttpSession>;
    using SpTimer = std::shared_ptr<Timer>;

    HttpServer(EventLoop* loop, const int port, const int iothreadnum, const int workthreadnum);
    ~HttpServer();

    void start();
private:
    TcpServer tcpserver_;
    ThreadPool threadpool_;

    std::mutex mutex_;

    std::map<SpTcpConnection, SpHttpSession> httpsessions_;
    std::map<SpTcpConnection, SpTimer> httptimers_;

    void handleMessage(const SpTcpConnection&, std::string&);
    void handleSendComplete(const SpTcpConnection&);
    void handleError(const SpTcpConnection&);
    void handleNewConnection(const SpTcpConnection&);
    void handleClose(const SpTcpConnection&);
};
}
#endif // HTTPSERVER_H
