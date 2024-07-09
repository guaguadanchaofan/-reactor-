#pragma once
#include "Dispatcher.h"
#include <sys/select.h>
class SelectDispatcher : public Dispatcher
{
public:
    SelectDispatcher(EventLoop *evloop) : Dispatcher(evloop)
    {
        FD_ZERO(&_readset);
        FD_ZERO(&_writeset);
        _name = "Select";
    }
    int add() override;
    int remove() override;
    int modify() override;
    int dispatch(int timeout = 0) override;

    ~SelectDispatcher()
    {
    }

private:
    void setFdSet();
    void clearFdSet();

private:
    fd_set _writeset;
    fd_set _readset;
    const int _MaxSize = 1024;
};