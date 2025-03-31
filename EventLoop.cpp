#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>

#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

// 保证一个线程只创建一个EventLoop
thread_local EventLoop *t_loopInThisThread = nullptr;

// 默认超时事件
const int kPollTimeMs = 10000;

// 创建wakeupfd，用于唤醒subloop处理channel
int createEventfd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("Failed in eventfd\n");
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
//   currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop created %p in thread %d\n", this, threadId_);
    if (t_loopInThisThread)
    {
        // 当前线程已经创建了EventLoop
        LOG_FATAL("Another EventLoop %p exists in this thread %d", this, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    // 设置回调
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading(); // 关注读事件
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

// 处理唤醒文件描述符的读事件
// 通过向 wakeupFd_ 写入数据，触发文件描述符的可读事件，使阻塞中的事件循环返回。
void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8\n", n);
    }
}

// 开启事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping.\n", this);

    while (!quit_)
    {
        activeChannels_.clear();

        // clientfd和wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for (Channel *channel : activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }
        doPendingFunctors(); // 唤醒subloop后，执行mainloop注册的回调
    }

    LOG_INFO("EventLoop %p stop looping.\n", this);
    looping_ = false;
}

// 退出事件循环
void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

// 在当前线程执行cb
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb); // queueInLoop(std::move(cb));
    }
}

// 把cb放入队列，唤醒loop所在线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // callingPendingFunctors_ = true
    // 正在执行回调时有新回调被注册，回调执行完后会阻塞在loop,唤醒以执行新的回调
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

// 发送一字节数据以唤醒subloop
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR("EventLoop::wakeup() writes %ld bytes instead of 8\n", n);
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    // 定义局部变量数组
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor &functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}
