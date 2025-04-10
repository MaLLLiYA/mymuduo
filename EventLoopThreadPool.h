#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "noncopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThreadPool(EventLoop *baseloop, const std::string nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    // 根据线程数量创建新线程与EventLoop
    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    // 轮询所有loops
    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    bool started() const { return started_; }

    const std::string name() const { return name_; }

private:
    EventLoop *baseLoop_; // EventLoop loop;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_; // 轮询时的下标
    // 智能指针管理EventLoopThread
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};