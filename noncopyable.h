#pragma once

// 继承自noncopyable的类可以构造和析构，不能拷贝构造与赋值
class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};