#include "TcpServer.h"
#include "Logger.h"
#include<functional>
EventLoop *checkLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("[%s:%s:%d]EventLoop is nullptr!", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,std::string &nameArg ,Option opt)
    : loop_(checkLoopNotNull(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceprot(loop,listenAddr,opt == kNoReusePort)),
    threadPoll_(new EventLoopThreadPoll(loop,name_)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));

}

TcpServer::~TcpServer()
{
}

void TcpServer::setThreadNum(int numThreads)
{
    threadPoll_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if(started_++ == 0) //防止一个Tcpserver对象被start多次
    {
        threadPoll_->start(threadInitCallback_);//启动底层的线程池
        loop_->runInloop(std::bind(&Acceprot::listen,acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
}
