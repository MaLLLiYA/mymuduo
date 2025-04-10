#include <memory>

#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), name), // threadFunc是成员函数，有this参数
      mutex_(),
      cond_(),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join(); // 等待子线程结束
    }
}

// 启动新线程，并获取新线程上的EventLoop指针
EventLoop *EventLoopThread::startLoop()
{
    thread_.start(); // 启动新线程
    EventLoop *loop = nullptr;
    {
        // 等待新线程: (1)loop创建完成 (2)初始化成员变量loop_
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        // loop_ != nullptr
        loop = loop_;
    }
    return loop;
}

// 在【新线程】中执行的线程函数
void EventLoopThread::threadFunc()
{
    EventLoop loop; // (1)loop创建
    if (callback_)
    {
        callback_(&loop); // std::function<void(EventLoop *)>
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop; // (2)初始化成员变量loop_
        cond_.notify_one();
    }

    loop.loop(); // Eventloop.loop() => Poller.poll()
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}