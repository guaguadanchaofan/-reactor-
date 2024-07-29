#include"Poller.h"
#include<stdlib.h>
#include"EpollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USR_POLL"))
    {
        return nullptr; //生成poll的实例
    }
    else
    {
        return new EpollPoller(loop); //生成epoll实例
    }
}