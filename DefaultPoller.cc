#include"Poller.h"
#include<stdlib.h>



Poller* Poller::newDefaultPoller(Channel* channel)
{
    if(::getenv("MUDUO_USR_POLL"))
    {
        return nullptr; //生成poll的实例
    }
    else
    {
        return nullptr; //生成epoll实例
    }
}