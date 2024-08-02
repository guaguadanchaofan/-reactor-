#include "Acceprot.h"
#include "Socket.h"
#include <sys/socket.h>
#include "Logger.h"
#include <errno.h>
#include "InetAddress.h"
#include <unistd.h>
static int createNonblocking()
{
    LOG_INFO("func = %s \n", __FUNCTION__);
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        LOG_FATAL("[%s:%s:%d]listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

Acceprot::Acceprot(EventLoop *loop, const InetAddress &listenaddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(createNonblocking()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReuserPort(reuseport);
    acceptSocket_.bindAddress(listenaddr);
    acceptChannel_.setReadCallback(std::bind(&Acceprot::handleRead, this));
}

Acceprot::~Acceprot()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceprot::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceprot::handleRead()
{
    InetAddress peerAddr;
    int cfd = acceptSocket_.accept(&peerAddr);
    if (cfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(cfd, peerAddr);//轮询找到subloop分发当前新客户端的channel
        }
        else
        {
            ::close(cfd);
        }
    }
    else
    {
        LOG_ERROR("[%s:%s:%d]accept socket error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE)
        {
            LOG_ERROR("[%s:%s:%d]socket reached limit\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}