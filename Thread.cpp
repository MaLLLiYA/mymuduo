#include <semaphore.h>

#include "Thread.h"
#include "CurrentThread.h"

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false), joined_(false), tid_(0), func_(std::move(func)), name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    thread_ = std::shared_ptr<std::thread>(new std::thread([&]()
                                                           {
        tid_=CurrentThread::tid();
        sem_post(&sem);
        // 执行线程函数
        func_(); }));

    // 等待获取到tid
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}