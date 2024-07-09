#include "Channel.h"

 Channel::Channel(int fd, ElemType event, HandelFunc writeFunc, HandelFunc readFunc, HandelFunc destroyFunc, void *arg)
 {
    _fd=fd;
    _events=(int)event;
    _writecallback=writeFunc;
    _readcallback=readFunc;
    _destroycallback=destroyFunc;
    _arg=arg;
 }

 void Channel::writeEventEnable( bool flag)
 {
    if (flag)
    {
        _events |= static_cast<int>(FDevent::readevent);
    }
    else
    {
        _events &=~static_cast<int>(FDevent::writevent);
    }
 }

bool Channel::iswriteEventEnable()
{
     if (_events | 0 == static_cast<int>(FDevent::writevent))
    {
        return true;
    }
    return false;
}