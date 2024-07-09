#pragma once
#include "Dispatcher.h"
#include <sys/poll.h>
#define MAX 1024
class PollDispatcher : public Dispatcher
{
public:
    PollDispatcher(EventLoop* evloop):Dispatcher(evloop)
    {
        _Maxfd = 0;
        _fds = new struct pollfd[_MaxNode];
        for (int i = 0; i < MAX; i++)
        {
            _fds[i].fd = -1;
            _fds[i].events = 0;
            _fds[i].revents = 0;
        }
        _name = "Poll";
    }
    int add() override;
    int remove() override;
    int modify() override;
    int dispatch(int timeout = 0) override;
    ~PollDispatcher()
    {
        delete[] _fds;
    }

private:
    int _Maxfd;
    struct pollfd *_fds;
    const int _MaxNode=1024;
};