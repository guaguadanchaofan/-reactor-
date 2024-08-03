#include "TcpServer.h"
#include "Logger.h"
#include<strings.h>
#include<functional>
EventLoop *checkLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("[%s:%s:%d]EventLoop is nullptr!", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,const std::string &nameArg ,Option opt)
    : loop_(checkLoopNotNull(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceprot(loop,listenAddr,opt == kNoReusePort)),
    threadPoll_(new EventLoopThreadPoll(loop,name_)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1),
    started_(0)
{
    // 当有先用户连接时，会执行TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));

}

TcpServer::~TcpServer()
{
    for(auto &itme : connections_)
    {
        TcpConnectionPtr conn(itme.second);
        itme.second.reset();

        
        conn->getLoop()->runInloop(std::bind(&TcpConnection::connectDestroy,conn));
    }
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


//有一个新的客户端连接，acceptor会执行这个回调函数
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop* ioloop = threadPoll_->getNextLoop(); //轮询算法选择subloop 管理channel
    char buf[64] = {0};
    snprintf(buf , sizeof buf , "-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n", name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

    //通过sockfd获取其绑定的本机ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local , sizeof local);
    socklen_t addrlen  = sizeof local;
    if(::getsockname(sockfd,(sockaddr*)&local,&addrlen) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr");
    }
    InetAddress localAddr(local);
    TcpConnectionPtr conn(new TcpConnection(ioloop,connName,sockfd,localAddr,peerAddr));
    connections_[connName] = conn;
    //下面的回调都是用户设置给Tcpserver->TcpConnection->Channel->poller->notify channel
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
    ioloop->runInloop(std::bind(&TcpConnection::connectEstablished,conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInloop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n",name_.c_str(),conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop* ioloop = conn->getLoop();
    loop_->queueInloop(std::bind(&TcpConnection::connectDestroy,conn));
}
