#pragma once

#include "noncopyable.h"

class InetAddress;

// 封装socketfd
class Socket : noncopyable
{
public:
    explicit Socket(int sockfd);

    ~Socket();

    int fd() const { return sockfd_; }

    void bindAddress(const InetAddress &localaddr);

    void listen();

    // 返回connfd，并将其地址写入peeraddr
    int accetp(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);

private:
    const int sockfd_;
};