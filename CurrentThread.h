#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    extern thread_local int t_cachedTid;

    void cacheTid(); // 避免频繁的切换用户态/内核态

    // 多线程环境中快速获取当前线程的ID
    inline int tid()
    { // 分支预测优化
        // t_cachedTid == 0的概率低，表达式的值为0的概率高
        // (t_cachedTid == 0)==0(条件不成立)的概率高
        // 返回值为t_cachedTid == 0
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}