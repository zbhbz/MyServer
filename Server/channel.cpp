#include "channel.h"
#include "sys/epoll.h"
#include <iostream>

void myserver::Channel::handleEvent(){
    if (events_ & EPOLLHUP) {
        std::cout << "event EPOLLHUP" << std::endl;
        closehandler_();
    } else if (events_ & (EPOLLIN | EPOLLPRI)) {
        std::cout << "event EPOLLIN | EPOLLPRI ..." << std::endl;
        readhandler_();
    } else if (events_ & EPOLLOUT) {
        std::cout << "event EPOLLOUT" << std::endl;
        writehandler_();
    } else {
        std::cout << "event error" << std::endl;
        errorhandler_();
    }
}
