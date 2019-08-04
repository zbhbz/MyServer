
#include "httpserver.h"
#include "timermanage.h"

namespace myserver {

HttpServer::HttpServer(EventLoop* loop, const int port, const int iothreadnum, const int workthreadnum) :
tcpserver_(loop, port, iothreadnum), threadpool_(workthreadnum){
    tcpserver_.setNewConnectionCallback(std::bind(&HttpServer::handleNewConnection, this, std::placeholders::_1));
    tcpserver_.setMessageCallback(std::bind(&HttpServer::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setSendCompleteCallback(std::bind(&HttpServer::handleSendComplete, this, std::placeholders::_1));
    tcpserver_.setCloseCallback(std::bind(&HttpServer::handleClose, this, std::placeholders::_1));
    tcpserver_.setErrorCallback(std::bind(&HttpServer::handleError, this, std::placeholders::_1));

    threadpool_.start();

    TimerManage::getInstance()->start();
}

HttpServer::~HttpServer() {
}

void HttpServer::start() {
    tcpserver_.start();
}

void HttpServer::handleMessage(const SpTcpConnection& sptcpconnection, std::string& msg) {
    SpHttpSession sphttpsession;
    SpTimer sptimer;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        sphttpsession = httpsessions_[sptcpconnection];
        sptimer = httptimers_[sptcpconnection];
    }
    sptimer->adjustInTimerManage(5000, Timer::TimerType::TIMER_ONCE, std::bind(&TcpConnection::shutdown, sptcpconnection));

    if (threadpool_.getThreadNum() > 0) {
        HttpRequestContext request;
        std::string response;
        if ((sphttpsession->parseHttpRequest(msg, request)) == false) {
            sphttpsession->httpError(400, "Bad request", request, response);
            sptcpconnection->send(response);
            return;
        }

        sptcpconnection->setAsyncProcessing(true);
        threadpool_.addTask([=](){
            std::string responsemsg;
            sphttpsession->httpProcess(request, responsemsg);
            sptcpconnection->send(responsemsg);
            if (sphttpsession->keepAlive() == false) {;}
        });
    } else {
        HttpRequestContext request;
        std::string response;
        if ((sphttpsession->parseHttpRequest(msg, request)) == false) {
            sphttpsession->httpError(400, "Bad request", request, response);
            sptcpconnection->send(response);
            return;
        }
        sphttpsession->httpProcess(request, response);
        sptcpconnection->send(response);
        if (sphttpsession->keepAlive() == false) {;}
    }
}

void HttpServer::handleSendComplete(const SpTcpConnection& sptcpconnection) {

}

void HttpServer::handleError(const SpTcpConnection& sptcpconnection) {
    std::unique_lock<std::mutex> lock(mutex_);
    httpsessions_.erase(sptcpconnection);
    httptimers_.erase(sptcpconnection);
}

void HttpServer::handleNewConnection(const SpTcpConnection& sptcpconnection) {
    SpHttpSession httpsession = std::make_shared<HttpSession>();
    SpTimer timer = std::make_shared<Timer>(5000, Timer::TimerType::TIMER_ONCE, std::bind(&TcpConnection::shutdown, sptcpconnection));
    timer->addToTimerManage();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        httpsessions_[sptcpconnection] = httpsession;
        httptimers_[sptcpconnection] = timer;
    }
}

void HttpServer::handleClose(const SpTcpConnection& sptcpconnection) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        httpsessions_.erase(sptcpconnection);
        httptimers_.erase(sptcpconnection);
        //std::cout << "http handleClose" << std::endl;
    }
}
}
