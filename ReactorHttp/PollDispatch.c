#include "Dispatcher.h"
#include <poll.h>
#define MAX 1024

// 初始化---poll select epoll的数据块
static void *pollinit();
// 增加
static int polladd(struct Channel *channel, struct EventLoop *EventLoop);
// 删除
static int pollremove(struct Channel *channel, struct EventLoop *EventLoop);
// 修改
static int pollmodify(struct Channel *channel, struct EventLoop *EventLoop);
// 事件检测
static int polldispatch(struct EventLoop *EventLoop, int timeout);
// 清除
static int pollclear(struct EventLoop *EventLoop);

struct PollData
{
    int Maxfd;
    struct pollfd _fds[MAX];
};

struct Dispatcher Polldispatch = {
    pollinit,
    polladd,
    pollremove,
    pollmodify,
    polldispatch,
    pollclear};

// static定义局部函数只在本文件生效
static void *pollinit()
{
    struct PollData *data = (struct PollData *)malloc(sizeof(struct PollData));
    data->Maxfd = 0;
    // 将数组初始化
    for (int i = 0; i < MAX; i++)
    {
        data->_fds[i].fd = -1;
        data->_fds[i].events = 0;
        data->_fds[i].revents = 0;
    }
    return data;
}

static int polladd(struct Channel *channel, struct EventLoop *EventLoop)
{
    struct PollData *data = (struct PollData *)EventLoop->disepatherData;
    // 获取事件值
    int event = 0;
    if (channel->_events & readevent)
    {
        event |= POLLIN;
    }
    if (channel->_events & writevent)
    {
        event |= POLLOUT;
    }
    int i = 0;
    // 将fd找到空位置插入进去
    for (; i < MAX; i++)
    {
        // 判断位置是否被占用
        if (data->_fds[i].fd == -1)
        {
            data->_fds->fd = channel->_fd;
            data->_fds->events = event;
            // 判断是否要更新最大fd数量
            data->Maxfd = i > data->Maxfd ? i : data->Maxfd;
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

static int pollremove(struct Channel *channel, struct EventLoop *EventLoop)
{
    struct PollData *data = (struct PollData *)EventLoop->disepatherData;
    // 获取事件值
    int i = 0;
    // 将fd删除
    for (; i < MAX; i++)
    {
        // 判断位置是否被占用
        if (data->_fds[i].fd == channel->_fd)
        {
            data->_fds->fd = -1;
            data->_fds->events = 0;
            break;
        }
    }
    //通过channel释放对应的tcpconnection资源
    channel->_destroycallback(channel->_arg);
    return 0;
}

static int pollmodify(struct Channel *channel, struct EventLoop *EventLoop)
{
    struct PollData *data = (struct PollData *)EventLoop->disepatherData;
    // 获取事件值
    int event = 0;
    if (channel->_events & readevent)
    {
        event |= POLLIN;
    }
    if (channel->_events & writevent)
    {
        event |= POLLOUT;
    }
    int i = 0;
    // 将fd找到
    for (; i < MAX; i++)
    {
        // 判断位置是否被占用
        if (data->_fds[i].fd == channel->_fd)
        {
            data->_fds->events = event;
            break;
        }
    }
    return 0;
}

static int polldispatch(struct EventLoop *EventLoop, int timeout)
{
    struct PollData *data = (struct PollData *)EventLoop->disepatherData;
    int count = poll(data->_fds, data->Maxfd + 1, 1000 * timeout);
    if (count == -1)
    {
        perror("poll");
        exit(0);
    }
    for (int i = 0; i < count; i++)
    {
        if (data->_fds[i].fd == -1)
        {
            continue;
        }
        if (data->_fds[i].revents & POLLIN)
        {
            //
            ActivateEvent(EventLoop,readevent,data->_fds[i].fd);
        }
        if (data->_fds[i].revents & POLLOUT)
        {
            //
            ActivateEvent(EventLoop,writevent,data->_fds[i].fd);
        }
    }
}

static int pollclear(struct EventLoop *EventLoop)
{
    struct PollData *data = (struct PollData *)EventLoop->disepatherData;
    // 释放创建的结构体
    free(data);
}
