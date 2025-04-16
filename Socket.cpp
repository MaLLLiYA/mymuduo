#include <unistd.h>      // close
#include <sys/types.h>   // bind,listen,accept
#include <sys/socket.h>  // bind,listen,accept,shutdown
#include <netinet/tcp.h> // TCP_NODELAY
#include <strings.h>     // bzero

#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"

Socket::Socket(int sockfd) : sockfd_(sockfd) {}

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr)
{
    if (::bind(sockfd_, (sockaddr *)localaddr.getSockAddr(), sizeof(sockaddr)) != 0)
    {
        LOG_FATAL("Socket::bindAddress error:sockfd=%d", sockfd_);
    }
}

void Socket::listen()
{
    if (::listen(sockfd_, 1024) != 0)
    {
        LOG_FATAL("Socket::listen error:sockfd=%d", sockfd_);
    }
}

// 返回connfd，并将其地址写入peeraddr
int Socket::accetp(InetAddress *peeraddr)
{
    socklen_t len;
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    int connfd = ::accept(sockfd_, (sockaddr *)&addr, &len);

    if (connfd > 0)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) != 0)
    {
        LOG_ERROR("Socket::shutdownWrite error");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    // bool to int
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}