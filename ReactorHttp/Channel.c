#include "Channel.h"

// channel初始化
struct Channel *initchannel(int fd, int event, HandelFunc writeFunc, HandelFunc readFunc, void *arg)
{
    struct Channel *Channel = (struct Channel *)malloc(sizeof(struct Channel));
    Channel->_fd = fd;
    Channel->_events = event;
    Channel->_readcallback = readFunc;
    Channel->_writecallback = writeFunc;
    Channel->_arg = arg;
    return Channel;
}

// 修改fd的写事件（检测or不检测）
void writeEventEnable(struct Channel *Channel, bool flag)
{
    if (flag)
    {
        Channel->_events |= writevent;
    }
    else
    {
        Channel->_events &= ~writevent;
    }
}

// 判断是否需要检测文件描述的写事件
bool iswriteEventEnable(struct Channel *Channel)
{
    if (Channel->_events | 0 == writevent)
    {
        return true;
    }
    return false;
}