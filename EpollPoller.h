#pragma once
#include "Poller.h"
#include <vector>
#include <sys/epoll.h>
#include "Timestamp.h"

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    // 重写基类的方法
    Timestamp poll(int timeoutsMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;
    //填写活跃的链接
    void fillActiveChannels(int numEvents,ChannelList* activeChannels);//填写活跃channel
    //更新channel通道
    void update(int operation , Channel * channel);
private:
    using EventList = std::vector<epoll_event>;
    int epollfd_;
    EventList events_;
};