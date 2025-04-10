#pragma once

#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>

#include "noncopyable.h"
#include "Thread.h"
class EventLoop;

// 绑定thread和loop
// 在thread中创建一个loop
class EventLoopThread : noncopyable
{
public:
    // 线程初始化回调
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();
    EventLoop *loop_;       // 指向运行在thread_中的loop
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};