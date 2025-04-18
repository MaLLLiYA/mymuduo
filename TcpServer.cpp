#include "TcpServer.h"
#include "Logger.h"

EventLoop *checkLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d main loop is null. \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     Option option)
    : loop_(checkLoopNotNull(loop)),
      ipPort_(listenAddr.toIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(),
      messageCallback_(),
      nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
        // 有新用户连接时，执行TcpServer::newConnection回调
        // std::function<void(int sockfd, const InetAddress &)>
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

// 设置subloop的个数
void TcpServer::setThreadNum(int numTHreads)
{
    threadPool_->setThreadNum(numTHreads);
}

// 启动服务器并监听
// （1）启动EventLoopThreadPool
// （2）监听新连接的到达
void TcpServer::start()
{
    // 确保TcpServer只启动一次
    if (started_++ == 0)
    {
        threadPool_->start(threadInitCallback_);    // 启动线程池

        loop_->runInLoop(
            std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {}