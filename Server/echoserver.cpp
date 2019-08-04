
#include "echoserver.h"
#include <iostream>
#include <string>

myserver::EchoServer::EchoServer(EventLoop* eventloop, const int port, const int threadnum) :
tcpserver_(eventloop, port, threadnum){
    tcpserver_.setNewConnectionCallback(std::bind(&EchoServer::handleNewConnection, this, std::placeholders::_1));
    tcpserver_.setMessageCallback(std::bind(&EchoServer::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setSendCompleteCallback(std::bind(&EchoServer::handleSendComplete, this, std::placeholders::_1));
    tcpserver_.setCloseCallback(std::bind(&EchoServer::handleClose, this, std::placeholders::_1));
    tcpserver_.setErrorCallback(std::bind(&EchoServer::handleError, this, std::placeholders::_1));
}

void myserver::EchoServer::start() {
    tcpserver_.start();
}

void myserver::EchoServer::handleMessage(const SpTcpConnection& sptcpconn, std::string& bufferin) {
    std::string str;
    str.swap(bufferin);
    str.insert(0, "reply :");
    sptcpconn->send(str);
}
void myserver::EchoServer::handleSendComplete(const SpTcpConnection&) {
    std::cout << "Message send complete" << std::endl;
}
void myserver::EchoServer::handleError(const SpTcpConnection&) {
    std::cout << "EchoServer error" << std::endl;
}
void myserver::EchoServer::handleNewConnection(const SpTcpConnection&) {
    std::cout << "New Connection Come in" << std::endl;
}
void myserver::EchoServer::handleClose(const SpTcpConnection&) {
    std::cout << "EchoServer conn close" << std::endl;
}
