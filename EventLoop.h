#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <functional> // function

#include "noncopyable.h"
#include "TimeStamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;

// 事件循环类，包含Channel和Poller
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    TimeStamp pollReturnTime() { return pollReturnTime_; }

    // 在当前线程执行cb
    void runInLoop(Functor cb);

    // 把cb放入队列，唤醒loop所在线程，执行cb
    void queueInLoop(Functor cb);

    void wakeup();

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_;
    std::atomic_bool quit_;

    // std::atomic_bool eventhandling_;

    const pid_t threadId_;

    TimeStamp pollReturnTime_;
    std::shared_ptr<Poller> poller_;

    int wakeupFd_; // 轮询选择一个subloop，并唤醒
    std::unique_ptr<Channel> wakeupChannel_;    // 监听一个wakeupChannel，用于被MainLoop唤醒

    ChannelList activeChannels_;
    // Channel *currentActiveChannel_; // assert

    std::mutex mutex_;
    std::atomic_bool callingPendingFunctors_;   // doPendingFunctors()正在执行回调
    std::vector<Functor> pendingFunctors_;
};