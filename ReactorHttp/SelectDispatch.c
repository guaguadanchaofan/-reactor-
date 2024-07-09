#include "Dispatcher.h"
#include <sys/select.h>
#include<stdio.h>
#include<stdlib.h>
#define MAX 1024

struct SelectData
{
    fd_set _writeset;
    fd_set _readset;
};

// 初始化---poll select epoll的数据块
static void *selectinit();
// 增加
static int selectadd(struct Channel *channel, struct EventLoop *EventLoop);
// 删除
static int selectremove(struct Channel *channel, struct EventLoop *EventLoop);
// 修改
static int selectmodify(struct Channel *channel, struct EventLoop *EventLoop);
// 事件检测
static int selectdispatch(struct EventLoop *EventLoop, int timeout);
// 清除
static int selectclear(struct EventLoop *EventLoop);
static void setFdSet(struct Channel *channel, struct SelectData *data);
static void clearFdSet(struct Channel *channel, struct SelectData *data);


struct Dispatcher Selectdispatch = {
    selectinit,
    selectadd,
    selectremove,
    selectmodify,
    selectdispatch,
    selectclear};

// static定义局部函数只在本文件生效
static void *selectinit()
{
    struct SelectData *data = (struct SelectData *)malloc(sizeof(struct SelectData));
    FD_ZERO(&data->_readset);
    FD_ZERO(&data->_writeset);
    return data;
}
static void setFdSet(struct Channel *channel, struct SelectData *data)
{
    if (channel->_events & readevent)
    {
        FD_SET(channel->_fd, &data->_readset);
    }
    if (channel->_events & writevent)
    {
        FD_SET(channel->_fd, &data->_writeset);
    }
}
static void clearFdSet(struct Channel *channel, struct SelectData *data)
{
    if (channel->_events & readevent)
    {
        FD_CLR(channel->_fd, &data->_readset);
    }
    if (channel->_events & writevent)
    {
        FD_CLR(channel->_fd, &data->_writeset);
    }
}
static int selectadd(struct Channel *channel, struct EventLoop *EventLoop)
{
    struct SelectData *data = (struct SelectData *)EventLoop->disepatherData;
    setFdSet(channel, data);
    return 0;
}

static int selectremove(struct Channel *channel, struct EventLoop *EventLoop)
{
    struct SelectData *data = (struct SelectData *)EventLoop->disepatherData;
    clearFdSet(channel, data);
    channel->_destroycallback(channel->_arg);
    return 0;
}

static int selectmodify(struct Channel *channel, struct EventLoop *EventLoop)
{
    struct SelectData *data = (struct SelectData *)EventLoop->disepatherData;
    setFdSet(channel, data);
    clearFdSet(channel, data);
    return 0;
}

static int selectdispatch(struct EventLoop *EventLoop, int timeout)
{
    struct SelectData *data = (struct SelectData *)EventLoop->disepatherData;
    struct timeval val;
    val.tv_sec=timeout;
    val.tv_usec=0;
    fd_set wset=data->_writeset;
    fd_set rset=data->_readset;
    int count = select(MAX, &wset ,&rset,NULL, &val);
    if (count == -1)
    {
        perror("select");
        exit(0);
    }
    for (int i = 0; i < MAX; i++)
    {
        if (FD_ISSET(i,&rset))
        {
            //
            ActivateEvent(EventLoop,readevent,i);
        }
        if (FD_ISSET(i,&wset))
        {
            //
            ActivateEvent(EventLoop,writevent,i);
        }
    }
}

static int selectclear(struct EventLoop *EventLoop)
{
    struct SelectData *data = (struct SelectData *)EventLoop->disepatherData;
    // 释放创建的结构体
    free(data);
    return 0;
}
