
#include "tcpserver.h"
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

const int myserver::TcpServer::MAXCONNECTION = 20000;

myserver::TcpServer::TcpServer(EventLoop* eventloop, const int port, const int iothreadnum) :
eventloop_(eventloop), conncount_(0), eventloopthreadpool_(eventloop, iothreadnum){
    socket_.setReuseAddr();
    socket_.bindAddr(port);
    socket_.setNonBlocking();
    socket_.listen();

    channel_.setFd(socket_.getFd());
    channel_.setReadHandler(std::bind(&TcpServer::onNewConnection, this));
    channel_.setErrorHandler(std::bind(&TcpServer::onConnectionError, this));
}

void myserver::TcpServer::start() {
    eventloopthreadpool_.startAllThread();

    channel_.setEvents(EPOLLIN | EPOLLET);
    eventloop_->addChannel(&channel_);
}

void setNonBlocking(int fd) {
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0) {
        perror("fcntl(fd, F_GETFL)");
        exit(-1);
    }
    if (fcntl(fd, F_SETFL, opts | O_NONBLOCK) < 0) {
        perror("fcntl(fd, F_SETFL, opts | O_NONBLOCK)");
        exit(-1);
    }
    //std::cout << "setNonBlocking success" << std::endl;
}

void myserver::TcpServer::onNewConnection() {
    //std::cout << "onNewConnection1" << std::endl;
    //std::cout << "handleRead tcp server fd:" << socket_.getFd() << std::endl;
    struct sockaddr_in clientAddr;
    int clientFd;
    while ((clientFd = socket_.accept(clientAddr)) > 0) {
        std::cout << "new client IP:" << inet_ntoa(clientAddr.sin_addr)
                  << " PORT:" << ntohs(clientAddr.sin_port) << std::endl;
        if (conncount_ >= MAXCONNECTION) {
            close(clientFd);
            continue;
        }
        setNonBlocking(clientFd);
        EventLoop* loop = eventloopthreadpool_.getNextLoop();

        std::shared_ptr<TcpConnection> spTcpConnection(new TcpConnection(loop, clientFd, clientAddr));
        spTcpConnection->setMessageCallback(messagecallback_);
        spTcpConnection->setSendCompleteCallback(sendcompletecallback_);
        spTcpConnection->setCloseCallback(closecallback_);
        spTcpConnection->setErrorCallback(errorcallback_);
        spTcpConnection->setConnectCleanUpCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
        {
            std::unique_lock<std::mutex> lock(mutex_);
            ++conncount_;
            conntable[clientFd] = spTcpConnection;
        }
        newconnectioncallback_(spTcpConnection);
        spTcpConnection->addChannelToEventLoop();
    }
}

void myserver::TcpServer::removeConnection(const std::shared_ptr<TcpConnection> spTcpConnection) {
    std::unique_lock<std::mutex> lock(mutex_);
    --conncount_;
    conntable.erase(spTcpConnection->getFd());
    //std::cout << conncount_ << "removeConnection" << spTcpConnection->getFd() << std::endl;
}

void myserver::TcpServer::onConnectionError() {
    std::cout << "unknow error" << std::endl;
    socket_.close();
}
