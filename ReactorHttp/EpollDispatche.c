#include "Dispatcher.h"
#include <sys/epoll.h>
#define MAX 500

// 初始化---poll select epoll的数据块
static void *epollinit();
// 增加
static int epolladd(struct Channel *channel, struct EventLoop *EventLoop);
// 删除
static int epollremove(struct Channel *channel, struct EventLoop *EventLoop);
// 修改
static int epollmodify(struct Channel *channel, struct EventLoop *EventLoop);
// 事件检测
static int epolldispatch(struct EventLoop *EventLoop, int timeout);
// 清除
static int epollclear(struct EventLoop *EventLoop);
static int epollctl(struct Channel *channel, struct EventLoop *EventLoop,int op);

struct EpollData
{
    int _epfd;
    struct epoll_event *_ev;
};

struct Dispatcher Epolldispatch = {
    epollinit,
    epolladd,
    epollremove,
    epollmodify,
    epolldispatch,
    epollclear};

// static定义局部函数只在本文件生效
static void *epollinit()
{
    struct EpollData *data = (struct EpollData *)malloc(sizeof(struct EpollData));
    data->_epfd = epoll_create(1);
    if(data->_epfd==-1)
    {
        perror("epoll_create");
        exit(0);
    }
    data->_ev = (struct epoll_event *)calloc(MAX, sizeof(struct epoll_event));
    return data;
}

static int epollctl(struct Channel *channel, struct EventLoop *EventLoop,int op)
{
    struct EpollData* data=(struct EpollData*)EventLoop->disepatherData;
    struct epoll_event ev;
    ev.data.fd=channel->_fd;
    if(channel->_events&readevent)
    {
        ev.events|=EPOLLIN;
    }
    if(channel->_events&writevent)
    {
        ev.events|=EPOLLOUT;
    }
    int ret = epoll_ctl(data->_epfd,op,channel->_fd,&ev);
    return ret;
}

static int epolladd(struct Channel *channel, struct EventLoop *EventLoop)
{
    int ret = epollctl(channel,EventLoop,EPOLL_CTL_ADD);
    if(ret == -1)
    {
        perror("EPOLL_CTL_ADD");
        exit(0);
    }
}

static int epollremove(struct Channel *channel, struct EventLoop *EventLoop)
{
    int ret = epollctl(channel,EventLoop,EPOLL_CTL_DEL);
    if(ret == -1)
    {
        perror("EPOLL_CTL_DEL");
        exit(0);
    }
}

static int epollmodify(struct Channel *channel, struct EventLoop *EventLoop)
{
    int ret = epollctl(channel,EventLoop,EPOLL_CTL_MOD);
    if(ret == -1)
    {
        perror("EPOLL_CTL_MOD");
        exit(0);
    }
}

static int epolldispatch(struct EventLoop *EventLoop, int timeout)
{
}

static int epollclear(struct EventLoop *EventLoop)
{
}
