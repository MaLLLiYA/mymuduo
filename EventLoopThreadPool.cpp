#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, const std::string nameArg)
    : baseLoop_(baseloop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // loop 在EventLoopThread::threadFunc()中创建，是栈上的对象，不需要手动释放
}

// 根据线程数量创建新线程与EventLoop
void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32] = {0};
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        // 创建EventLoopThread线程对象
        EventLoopThread *t = new EventLoopThread(cb, buf);

        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }

    // 未设置线程数量，仅一个主线程
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

// 轮询所有loops
EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;
    if (!loops_.empty())
    {
        loop = loops_[next_]; // default next_ = 0
        ++next_;
        if (next_ >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}
