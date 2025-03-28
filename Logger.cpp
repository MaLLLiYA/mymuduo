#include <iostream>

#include "Logger.h"
#include "TimeStamp.h"

// 获取实例
Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

// 设置日志等级
void Logger::setLogLevel(int level)
{
    logLevel_ = level;
}

// 输出日志 [日志等级] time msg
void Logger::log(std::string msg)
{
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;
    }
    std::cout << TimeStamp::now().toString() << " : " << msg << std::endl;
}