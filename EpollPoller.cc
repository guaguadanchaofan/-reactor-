#include "EpollPoller.h"
#include "Logger.h"
#include <errno.h>
#include <unistd.h>
#include "Channel.h"
#include <string.h>


// channel 未添加到poller中
const int kNew = -1;
// channel 已经添加到poller中
const int kAdded = 1;
// channel 从poller中删除
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop), // 委托构造  初始化从基类继承的成员
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}
EpollPoller::~EpollPoller()
{
    close(epollfd_);
}

// 给所有io复用保留统一的接口
Timestamp EpollPoller::poll(int timeoutsMs, ChannelList *ActiveChannels)
{
    LOG_INFO("func = %s -> fd total count:%lu\n", __FUNCTION__,channels_.size());
    int numEvents = epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutsMs);
    int saveError = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0)
    {
        LOG_INFO("%d events happened\n",numEvents);
        fillActiveChannels(numEvents,ActiveChannels); //传入活动的channel列表
        if(numEvents == events_.size())
        {
            events_.resize(events_.size()*2); // 二倍扩容
        }
    }
    else if(numEvents == 0)
    {
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }
    else
    {
        if(saveError != EINTR)
        {
            errno = saveError;
            LOG_ERROR("EpollPoller::poll() error");
        }
    }
    return now;
}

// channel update remove -> EventLoop updateChannel  remove channel -> Poller updere remove
void EpollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("func = %s fd = %d events = %d index = %d \n", __FUNCTION__, channel->fd(), channel->events(), channel->index());
    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void EpollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channels_.erase(fd);
    LOG_INFO("func = %s fd = %d \n", __FUNCTION__, channel->fd());
    channel->set_index(kNew);
}

// 填写活跃的链接
void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels)
{
    for(int i = 0 ; i < numEvents ; ++i)
    {
        Channel* channel =static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }

} // 填写活跃channel

// 更新channel通道 //add/mod/delete
void EpollPoller::update(int operation, Channel *channel)
{
    epoll_event evs;
    memset(&evs, 0, sizeof evs);
    evs.data.fd = channel->fd();
    evs.events = channel->events();
    evs.data.ptr = channel;
    int fd = channel->fd();
    if (epoll_ctl(epollfd_, operation, fd, &evs) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}