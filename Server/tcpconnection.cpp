
#include "tcpconnection.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#define BUFFSIZE 4096

int readn(int fd, std::string& bufferin) {
    int nbyte = 0;
    int ressum = 0;
    char buffer[BUFFSIZE];
    for (;;) {
        nbyte = read(fd, buffer, BUFFSIZE);
        if (nbyte > 0) {
            ressum += nbyte;
            bufferin.append(buffer, nbyte);
            if (nbyte < BUFFSIZE) {
                return ressum;
            } else {
                continue;
            }
        } else if (nbyte < 0) {
            if (errno == EAGAIN) {
                return ressum;
            } else if (errno == EINTR) {
                continue;
            } else {
                perror("readn error");
                return -1;
            }
        } else { // nbyte == 0 client -> FIN
            return 0;
        }
    }
}

int writen(int fd, std::string& bufferout) {
    int nbyte = 0;
    int ressum = 0;
    int len = bufferout.size();
    if (len >= BUFFSIZE) {
        len = BUFFSIZE;
    }
    for (;;) {
        nbyte = write(fd, bufferout.c_str(), len);
        if (nbyte > 0) {
            ressum += nbyte;
            bufferout.erase(0, nbyte);
            len = bufferout.size();
            if (len >= BUFFSIZE) {
                len = BUFFSIZE;
            }
            if (len == 0) {
                return ressum;
            }
        } else if (nbyte < 0) {
            if (errno == EAGAIN) {
                return ressum;
            } else if (errno == EINTR) {
                continue;
            } else if (errno == EPIPE) {
                perror("write error -- client RST");
                return -1;
            } else {
                perror("write error");
                return -1;
            }
        } else {
            return 0;
        }
    }
}

myserver::TcpConnection::TcpConnection(EventLoop* eventloop, int fd, const struct sockaddr_in& addr) :
eventloop_(eventloop), fd_(fd), clientaddr_(addr), channel_(new Channel()),
halfclose_(false), disconnected_(false), asyncprocessing_(false){
    channel_->setFd(fd_);
    channel_->setEvents(EPOLLIN | EPOLLET);
    channel_->setReadHandler(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteHandler(std::bind(&TcpConnection::handleWrite, this));
    channel_->setErrorHandler(std::bind(&TcpConnection::handleError, this));
    channel_->setCloseHandler(std::bind(&TcpConnection::handleClose, this));
}

myserver::TcpConnection::~TcpConnection() {
    eventloop_->removeChannel(channel_.get());
    close(fd_);
}

void myserver::TcpConnection::addChannelToEventLoop(){
    eventloop_->addTask(std::bind(&EventLoop::addChannel, eventloop_, channel_.get()));
}

void myserver::TcpConnection::send(const std::string& str){
    bufferout_ += str;
    // loop is a io thread which is used to send data
    if (eventloop_->getThreadId() == std::this_thread::get_id()) {
        sendInLoop();
    } else {
        asyncprocessing_ = false;
        eventloop_->addTask(std::bind(&TcpConnection::sendInLoop, shared_from_this()));
    }
}

void myserver::TcpConnection::sendInLoop(){
    if (disconnected_) {return;}
    int res = writen(fd_, bufferout_);
    if (res > 0) {
        uint32_t events = channel_->getEvents();
        if (bufferout_.size() > 0) {
            channel_->setEvents(events | EPOLLOUT);
            eventloop_->updateChannel(channel_.get());
        } else {
            channel_->setEvents(events & (~EPOLLOUT));
            sendcompletecallback_(shared_from_this());
            if (halfclose_)
                handleClose();
        }
    } else if (res < 0) {
        handleError();
    } else {
        handleClose();
    }
}

void myserver::TcpConnection::shutdown() {
    if (eventloop_->getThreadId() == std::this_thread::get_id()) {
        shutdownInLoop();
    } else {
        eventloop_->addTask(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void myserver::TcpConnection::shutdownInLoop(){
    if (disconnected_) return;
    closecallback_(shared_from_this());
    eventloop_->addTask(std::bind(connectcleanupcallback_, shared_from_this()));
    disconnected_ = true;
}

void myserver::TcpConnection::handleRead(){
    //std::cout << "handleRead tcp conn fd:" << fd_ << std::endl;
    int res = readn(fd_, bufferin_);
    if (res > 0) {
        messagecallback_(shared_from_this(), bufferin_);
    } else if (res == 0) {
        handleClose();
    } else {
        handleError();
    }
}
void myserver::TcpConnection::handleWrite(){
    int res = writen(fd_, bufferout_);
    if (res > 0) {
        uint32_t events = channel_->getEvents();
        if (bufferout_.size() > 0) {
            channel_->setEvents(events | EPOLLOUT);
            eventloop_->updateChannel(channel_.get());
        } else {
            channel_->setEvents(events & (~EPOLLOUT));
            sendcompletecallback_(shared_from_this());
            if (halfclose_)
                handleClose();
        }
    } else if (res < 0) {
        handleError();
    } else {
        handleClose();
    }
}

void myserver::TcpConnection::handleError(){
    if (disconnected_) return;
    errorcallback_(shared_from_this());
    eventloop_->addTask(std::bind(connectcleanupcallback_, shared_from_this()));
    disconnected_ = true;
}

void myserver::TcpConnection::handleClose(){
    if (disconnected_) return;
    if (!bufferin_.empty() || !bufferout_.empty() || asyncprocessing_) {
        halfclose_ = true;
        if (bufferin_.size() > 0) {
            messagecallback_(shared_from_this(), bufferin_);
        }
    } else {
        //std::cout << "TcpConnection handleClose" << std::endl;
        eventloop_->addTask(std::bind(connectcleanupcallback_, shared_from_this()));
        closecallback_(shared_from_this());
        disconnected_ = true;
    }
}
