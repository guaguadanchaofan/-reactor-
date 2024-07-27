#include"Poller.h"
#include<stdlib.h>



Poller* Poller::newDefaultPoller(Channel* channel)
{
    if(::getenv("MUDUO_USR_POLL"))
    {
        return nullptr;
    }
    else
    {
        return nullptr;
    }
}