#pragma once

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "TimeStamp.h"

class Channel;
class EventLoop;

// 多路事件分发器的核心IO复用模块
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);

    virtual ~Poller() = default;

    // 给所有IO复用保留统一的接口
    virtual TimeStamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    virtual bool hasChannel(Channel *channel) const;

    // 获取默认的IO复用的具体实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    // mapd的key表示sockfd
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_;
};