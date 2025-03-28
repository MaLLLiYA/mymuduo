#pragma once
#include <string>
#include <iostream>
class TimeStamp
{
public:
    TimeStamp();

    explicit TimeStamp(int64_t microSecondsSinceEpoch);

    // 获取当前时间
    static TimeStamp now();

    // 转换为字符串
    std::string toString() const;

private:
    int64_t microSecondsSinceEpoch_;
};