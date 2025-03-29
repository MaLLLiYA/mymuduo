#pragma once

#include <vector>
#include <sys/epoll.h> // epoll_event

#include "Poller.h"
#include "TimeStamp.h"

class Channel;
class EventLoop;

/*
epoll_create
epoll_ctl
epoll_wait
*/

// 封装epoll
class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    // 重写Poller虚函数
    TimeStamp poll(int timeoutMs, ChannelList *activeChannels) override;

    void updateChannel(Channel *channel) override;

    void removeChannel(Channel *channel) override;

private:
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    // 更新channel epoll_ctl add/mod/del
    void update(int operation, Channel *channel);

    static const int kInitEventListSize = 16;
    using EventList = std::vector<epoll_event>;
    EventList events_;
    int epollfd_;
};