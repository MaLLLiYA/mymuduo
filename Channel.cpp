#include <sys/epoll.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

// 初始化静态成员
const int kNoneEvent = 0;
const int KReadEvent = EPOLLIN | EPOLLPRI;
const int kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

// 通过所属的EventLoop，删除当前channel
void Channel::remove()
{
    // TODO
    // loop_->removeChannel(this);
}

// 当改变channel负责的events事件后，在poller中更改相应的事件 epoll_ctl
void Channel::update()
{
    // 通过所属的EventLoop，调用poller的相应方法
    // TODO
    // loop_->updateChannel(this);
}

// 处理事件，调用回调
void Channel::handleEvent(TimeStamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock(); // 提升为强引用
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// 根据poller通知的channel发生的具体事件，调用具体的回调
void Channel::handleEventWithGuard(TimeStamp receiveTIme)
{
    LOG_INFO("channel handleEvent revents:%d\n", revents_);
    // close
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }
    // error
    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            errorCallback_();
        }
    }
    // read
    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTIme);
        }
    }
    // write
    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}
