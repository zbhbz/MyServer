
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

myserver::Socket::Socket() {
    serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd_ == -1) {
        std::cout << "socket create fail" << std::endl;
        exit(-1);
    }
    std::cout << "server socket create success" << std::endl;
}

myserver::Socket::~Socket() {
    ::close(serverFd_);
    std::cout << "server close success" << std::endl;
}

void myserver::Socket::setReuseAddr() {
    int on = 1;
    setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    std::cout << "setReuseAddr success" << std::endl;
}

void myserver::Socket::setNonBlocking() {
    int opts = fcntl(serverFd_, F_GETFL);
    if (opts < 0) {
        perror("fcntl(serverFd_, F_GETFL)");
        exit(-1);
    }
    if (fcntl(serverFd_, F_SETFL, opts | O_NONBLOCK) < 0) {
        perror("fcntl(serverFd_, F_SETFL, opts | O_NONBLOCK)");
        exit(-1);
    }
    std::cout << "setNonBlocking success" << std::endl;
}

bool myserver::Socket::bindAddr(int port) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    int res = bind(serverFd_, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (res == -1) {
        ::close(serverFd_);
        perror("server bindAddr fail");
        exit(-1);
    }
    std::cout << "server bindAddr success" << std::endl;
    return true;
}

bool myserver::Socket::listen() {
    if (::listen(serverFd_, 8192) < 0) {
        ::close(serverFd_);
        perror("server listenServer fail");
        exit(-1);
    }
    std::cout << "server listen success" << std::endl;
    return true;
}

int myserver::Socket::accept(struct sockaddr_in &clientAddr) {
    socklen_t len = sizeof(clientAddr);
    int clientFd_ = ::accept(serverFd_, (struct sockaddr*)&clientAddr, &len);
    if (clientFd_ < 0) {
        if (errno == EAGAIN) return 0;
        return clientFd_;
    }
    //std::cout << "server acceptClient success" << std::endl;
    return clientFd_;
}

bool myserver::Socket::close() {
    ::close(serverFd_);
    std::cout << "server close success" << std::endl;
    return true;
}
