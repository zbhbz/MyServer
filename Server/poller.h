#ifndef POLLER_H
#define POLLER_H

#include "channel.h"
#include <mutex>
#include <vector>
#include <map>
#include <sys/epoll.h>

namespace myserver {

class Poller {
public:
    using ChannelList = std::vector<Channel*>;
    using EventList = std::vector<struct epoll_event>;
    using ChannelMap = std::map<int, Channel*>;

    Poller();
    ~Poller();

    std::mutex mutex_;

    int epfd_;

    EventList eventlist_;

    // used to check if the channel is exist
    ChannelMap channelmap_;

    void poll(ChannelList& activeChannelList);

    void addChannel(Channel* c);
    void removeChannel(Channel* c);
    void updateChannel(Channel* c);
};
}
#endif // POLLER_H
