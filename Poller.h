#pragma once
#include <vector>
#include <unordered_map>
#include "Timestamp.h"
#include "noncopyable.h"
class Channel;
class EventLoop;
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;
    Poller(EventLoop *loop);
    virtual ~Poller();
    // 给所有io复用保留统一的接口
    virtual Timestamp poll(int timeoutsMs, ChannelList *ActiveChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    bool hasChannel(Channel *channel) const;

    // 获取具体io复用的实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    // key表示fd value表示对应的channel
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    EventLoop *ownerloop_;
};