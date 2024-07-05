#pragma once
#include "Channel.h"
#include "EventLoop.h"

struct Dispatcher
{
    // 初始化---poll select epoll的数据块
    void *(*init)();
    // 增加
    int (*add)(struct Channel *channel, struct EventLoop *EventLoop);
    // 删除
    int (*remove)(struct Channel *channel, struct EventLoop *EventLoop);
    // 修改
    int (*modify)(struct Channel *channel, struct EventLoop *EventLoop);
    // 事件检测
    int (*dispatch)(struct EventLoop *EventLoop, int timeout);
    // 清除
    int (*clear)(struct EventLoop *EventLoop);
};