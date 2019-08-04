#include "poller.h"
#include <iostream>
#include <stdlib.h> // exit
#include <stdio.h> // perror
#include <sys/epoll.h>
#include <unistd.h> // close

#define MAXEVENTNUM 4096
#define TIMEOUT 1000

myserver::Poller::Poller() : epfd_(-1), eventlist_(MAXEVENTNUM) {
    epfd_ = epoll_create(256);
    if (epfd_ == -1) {
        perror("epoll create fail");
        exit(-1);
    }
    std::cout << "poll create success" << std::endl;
}

myserver::Poller::~Poller() {
    close(epfd_);
}

void myserver::Poller::poll(ChannelList& activeChannelList) {
    int num = epoll_wait(epfd_, &*eventlist_.begin(), static_cast<int>(eventlist_.capacity()), TIMEOUT);
    if (num > 0) {
        for (int i = 0; i < num; ++ i) {
            uint32_t events = eventlist_[i].events;
            Channel *ch = static_cast<Channel*>(eventlist_[i].data.ptr);
            int fd = ch->getFd();
            ChannelMap::const_iterator it;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                it = channelmap_.find(fd);
            }
            if (it != channelmap_.end()) {
                ch->setEvents(events);
                activeChannelList.push_back(ch);
            } else {
                std::cout << "can not find channel" << std::endl;
            }
        }
    } else if (num == -1) {
        perror("poll error");
    }
}

void myserver::Poller::addChannel(Channel* channel) {
    int fd = channel->getFd();
    struct epoll_event event;
    event.events = channel->getEvents();
    event.data.ptr = channel;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        channelmap_[fd] = channel;
    }

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event) == -1) {
        perror("epoll_ctl EPOLL_CTL_ADD fail");
        exit(-1);
    }
}

void myserver::Poller::updateChannel(Channel* channel) {
    int fd = channel->getFd();
    struct epoll_event event;
    event.events = channel->getEvents();
    event.data.ptr = channel;
    if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event) == -1) {
        perror("epoll_ctl EPOLL_CTL_MOD fail");
        exit(-1);
    }
}

void myserver::Poller::removeChannel(Channel* channel) {
    int fd = channel->getFd();
    struct epoll_event event;
    event.events = channel->getEvents();
    event.data.ptr = channel;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        channelmap_.erase(fd);
    }
    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, &event) == -1) {
        perror("epoll_ctl EPOLL_CTL_DEL fail");
        exit(-1);
    }
}
