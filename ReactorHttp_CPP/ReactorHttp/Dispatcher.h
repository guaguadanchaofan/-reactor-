#pragma once
#include <string>
#include "Channel.h"
#include "EventLoop.h"
#include "Dispatcher.h"
using namespace std;

class EventLoop;

class Dispatcher
{
public:
    Dispatcher()
    {
    }
    Dispatcher(EventLoop *EventLoop)
        : _EventLoop(EventLoop)
    {
    }
    virtual int add() = 0;
    virtual int remove() = 0;
    virtual int modify() = 0;
    virtual int dispatch(int timeout = 0) = 0;
    inline void setChannel(Channel *Channel)
    {
        _Channel = Channel;
    }
    virtual ~Dispatcher();

protected:
    Channel *_Channel;
    EventLoop *_EventLoop;
    string _name = string();
};