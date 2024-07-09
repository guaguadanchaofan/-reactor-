#include "PollDispatch.h"

int PollDispatcher::add()
{
    // 获取事件值
    int event = 0;
    if (_Channel->getevents() & (int)FDevent::readevent)
    {
        event |= POLLIN;
    }
    if (_Channel->getevents() & (int)FDevent::readevent)
    {
        event |= POLLOUT;
    }
    int i = 0;
    // 将fd找到空位置插入进去
    for (; i < MAX; i++)
    {
        // 判断位置是否被占用
        if (_fds[i].fd == -1)
        {
            _fds->fd = _Channel->getfd();
            _fds->events = event;
            // 判断是否要更新最大fd数量
            _Maxfd = i > _Maxfd ? i : _Maxfd;
            break;
        }
    }
    // 判断是否数组满？
    if (i > MAX)
    {
        return -1;
    }
    return 0;
}
int PollDispatcher::remove()
{

    int i = 0;
    for (; i < MAX; i++)
    {
        if (_fds[i].fd == _Channel->getfd())
        {
            _fds->fd = -1;
            _fds->events = 0;
            break;
        }
    }
    // 通过channel释放对应的tcpconnection资源
    _Channel->_destroycallback(const_cast<void*>(_Channel->getarg()));
    return 0;
}
int PollDispatcher::modify()
{
    int event = 0;
    if (_Channel->getevents() & (int)FDevent::readevent)
    {
        event |= POLLIN;
    }
    if (_Channel->getevents() & (int)FDevent::writevent)
    {
        event |= POLLOUT;
    }
    int i = 0;
    // 将fd找到
    for (; i < MAX; i++)
    {
        // 判断位置是否被占用
        if (_fds[i].fd == _Channel->getfd())
        {
            _fds->events = event;
            break;
        }
    }
    return 0;
}
int PollDispatcher::dispatch(int timeout = 0)
{
    int count = poll(_fds, _Maxfd + 1, 1000 * timeout);
    if (count == -1)
    {
        perror("poll");
        exit(0);
    }
    for (int i = 0; i < count; i++)
    {
        if (_fds[i].fd == -1)
        {
            continue;
        }
        if (_fds[i].revents & POLLIN)
        {
            //
            ActivateEvent(EventLoop, (int)FDevent::readevent, _fds[i].fd);
        }
        if (_fds[i].revents & POLLOUT)
        {
            //
            ActivateEvent(EventLoop,(int)FDevent:: writevent, _fds[i].fd);
        }
    }
}