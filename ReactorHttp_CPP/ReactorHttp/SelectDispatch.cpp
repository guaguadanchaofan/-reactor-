#include "SelectDispatch.h"

#include <stdio.h>
#include <stdlib.h>
#define MAX 1024

void SelectDispatcher::setFdSet()
{
    if (_Channel->getevents() & (int)FDevent::readevent)
    {
        FD_SET(_Channel->getfd(), &_readset);
    }
    if (_Channel->getevents() & (int)FDevent::writevent)
    {
        FD_SET(_Channel->getfd(), &_writeset);
    }
}

void SelectDispatcher::clearFdSet()
{
    if (_Channel->getevents() & (int)FDevent::readevent)
    {
        FD_CLR(_Channel->getfd(), &_readset);
    }
    if (_Channel->getevents() & (int)FDevent::writevent)
    {
        FD_CLR(_Channel->getfd(), &_writeset);
    }
}

int SelectDispatcher::add()
{
    if (_Channel->getfd() >= _MaxSize)
    {
        return -1;
    }
    setFdSet();
}

int SelectDispatcher::remove()
{
    clearFdSet();
    _Channel->_destroycallback(const_cast<void *>(_Channel->getarg()));
}

int SelectDispatcher::modify()
{
    setFdSet();
    clearFdSet();
}

int SelectDispatcher::dispatch(int timeout = 0)
{
    struct timeval val;
    val.tv_sec = timeout;
    val.tv_usec = 0;
    fd_set wset = _writeset;
    fd_set rset = _readset;
    int count = select(_MaxSize, &wset, &rset, NULL, &val);
    if (count == -1)
    {
        perror("select");
        exit(0);
    }
    for (int i = 0; i < _MaxSize; ++i)
    {
        if (FD_ISSET(i, &rset))
        {
            //
            ActivateEvent(EventLoop, (int)FDevent::readevent, i);
        }
        if (FD_ISSET(i, &wset))
        {
            //
            ActivateEvent(EventLoop, (int)FDevent::writevent, i);
        }
    }
}
