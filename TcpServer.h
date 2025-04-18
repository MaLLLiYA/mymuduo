#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <atomic>

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include "InetAddress.h"

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop *loop,
              const InetAddress &listenAddr,
              const std::string &nameArg,
              Option option = kNoReusePort);
    ~TcpServer();
    
    // 设置subloop的个数
    void setThreadNum(int numTHreads);

    // 启动服务器并监听
    // （1）启动EventLoopThreadPool
    // （2）监听新连接的到达
    void start();

    // 设置回调函数
    void setTHreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_; // 用户创建的baseloop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_;
};