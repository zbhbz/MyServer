#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include "eventloop.h"
#include "tcpserver.h"
#include "tcpconnection.h"

namespace myserver {
class EchoServer {
public:
    using SpTcpConnection = std::shared_ptr<TcpConnection>;

    EchoServer(EventLoop* eventloop, const int port, const int threadnum);

    void start();
private:
    TcpServer tcpserver_;

    void handleMessage(const SpTcpConnection&, std::string&);
    void handleSendComplete(const SpTcpConnection&);
    void handleError(const SpTcpConnection&);
    void handleNewConnection(const SpTcpConnection&);
    void handleClose(const SpTcpConnection&);
};
}
#endif ECHOSERVER_H
