#include <errno.h>  // errno
#include <unistd.h> // close
#include <string.h> // memset

#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

// 表示状态
// channel未添加到Poller
const int kNew = -1; // channel.index_=-1
// channel已添加到Poller
const int kAdded = 1;
// channel已从Poller删除
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create1 error: %d\n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

// 重写Poller虚函数
TimeStamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {}

// Channel::update->EventLoop::updateChannel(virtual)->Poller::updateChannel
// 根据index判断如何更新
void EPollPoller::updateChannel(Channel *channel)
{
    int index = channel->index();
    if (index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        // add
        if (index == kNew)
        {
            // 添加到map
            channels_[fd] = channel; // unordered_map<int, Channel *>
        }
        channel->set_index(kAdded);
        // 添加到epoll
        update(EPOLL_CTL_ADD, channel);
    }
    else // index = kAdded
    {
        int fd = channel->fd();

        // 对事件不感兴趣
        if (channel->isNoneEvent())
        {
            // del
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            // mod
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel) {}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {}

// 更新channel epoll_ctl add/mod/del
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));

    int fd = channel->fd();
    event.events = channel->events();
    event.data.ptr = channel;
    event.data.fd = fd;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error: %d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error: %d\n", errno);
        }
    }
}