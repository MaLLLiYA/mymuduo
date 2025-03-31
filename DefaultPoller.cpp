#include <stdlib.h> // getenv

#include "Poller.h"
#include "EPollPoller.h"

// 获取默认的IO复用的具体实现
Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
    {
        // TODO POLL实例
        return nullptr;
    }
    else
    {
        // TODO EPOLL实例
        return new EPollPoller(loop);
    }
}