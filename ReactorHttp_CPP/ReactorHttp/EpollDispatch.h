#pragma once
#include "Dispatcher.h"
#include <sys/epoll.h>

class EpollDispatcher : public Dispatcher
{
public:
    EpollDispatcher(EventLoop* evloop):Dispatcher(evloop)
    {
        _epfd=epoll_create(1);
        _ev = new struct epoll_event[_MAXNode];
        _name="Epoll";
    }
    int add() override;
    int remove() override;
    int modify() override;
    int dispatch(int timeout = 0) override;
    ~EpollDispatcher()
    {
        close(_epfd);
        delete[]_ev;
    }
private:
int epollctl(int op);
private:
    int _epfd;
    struct epoll_event *_ev;
    const int _MAXNode = 520;
};