#include "EpollDispatch.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "Log.h"

int EpollDispatcher::add()
{
    int ret =  (EPOLL_CTL_ADD);
    if (ret == -1)
    {
        perror("EPOLL_CTL_ADD");
        exit(0);
    }
}

int EpollDispatcher::dispatch(int timeout = 0)
{
    int count = epoll_wait(_epfd, _ev, _MAXNode, 1000 * timeout);
    if (count == -1)
    {
        perror("epoll_wait");
        exit(0);
    }
    for (int i = 0; i < count; i++)
    {
        int events = _ev[i].events;
        int fd = _ev[i].data.fd;
        if (events & EPOLLERR || events & EPOLLHUP)
        {
            // 对方断开连接 删除fd；
            // epollremove(channel,evloop);
            continue;
        }
        if (events & EPOLLIN)
        {
            //
            ActivateEvent(EventLoop, (int)FDevent::readevent, fd);
        }
        if (events & EPOLLOUT)
        {
            //
            ActivateEvent(EventLoop, (int)FDevent::writevent, fd);
        }
    }
}

int EpollDispatcher::remove()
{

    int ret = epollctl(EPOLL_CTL_DEL);
    if (ret == -1)
    {
        perror("EPOLL_CTL_DEL");
        exit(0);
    }
    _Channel->_destroycallback(_Channel->getarg());
    return 0;
}

int EpollDispatcher::modify()
{
    int ret = epollctl(EPOLL_CTL_MOD);
    if (ret == -1)
    {
        perror("EPOLL_CTL_MOD");
        exit(0);
    }
}

int EpollDispatcher::epollctl(int op)
{
    struct epoll_event ev;
    ev.data.fd = _Channel->getfd();
    int events = 0;
    if (_Channel->getevents() & (int)FDevent::readevent)
    {
        events |= EPOLLIN;
    }
    if (_Channel->getevents() &  (int)FDevent::writevent)
    {
        events |= EPOLLOUT;
    }
    ev.events = events;
    Debug("epfd:%d  ,op:%d,  fd:%d  ", _epfd, op, _Channel->getfd());
    int ret = epoll_ctl(_epfd, op,  _Channel->getfd(), &ev);
    return ret;
}
