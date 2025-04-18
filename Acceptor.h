#pragma once

#include <functional>

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

class InetAddress;
class EventLoop;


//用于 baseLoop 中，对 listen fd 及其相关方法进行封装
//监听新连接到达、接受新连接、分发连接给 subLoop 
class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }
    void listen();

    bool listening() { return listening_; }

private:
    // listenfd有事件发生
    void handleRead();
    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};