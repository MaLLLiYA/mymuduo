#pragma once

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "TimeStamp.h"

class EventLoop; // 未定义变量，仅使用类型，使用前置申明
/*
封装了socketfd和感兴趣的event,EPOLLIN,EPOLLOUT等，
还绑定了poller返回的具体事件
一个Channel属于一个EventLoop
*/
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(TimeStamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // 处理事件，调用回调
    void handleEvent(TimeStamp receiveTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止channel被移除后仍执行回调
    void tie(const std::shared_ptr<void> &);

    int fd() { return fd_; }
    int events() { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    // 设置fd相应的事件状态
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }

    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }

    void enableReading()
    {
        events_ |= KReadEvent;
        update();
    }
    
    void disableReading()
    {
        events_ &= ~KReadEvent;
        update();
    }

    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & KReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop *ownerLoop() { return loop_; }

    // 在channel所属的EventLoop中删除当前channel
    void remove();

private:
    void update();
    // 根据poller通知的channel发生的具体事件，调用具体的回调
    void handleEventWithGuard(TimeStamp receiveTIme);

    static const int kNoneEvent;
    static const int KReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;
    const int fd_;
    int events_;  // 注册感兴趣的事件
    int revents_; // poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    // Channel能够获知fd发生的具体事件，负责调用回调
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};