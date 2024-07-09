#pragma once
#include "Dispatcher.h"
#include "ChannelMap.h"
#include <stdbool.h>
#include <pthread.h>
extern struct Dispatcher Epolldispatch;
extern struct Dispatcher Polldispatch;
extern struct Dispatcher Selectdispatch;

// 处理channel的方式
enum
{
    ADD,
    DELETE,
    MODIFY
};

// 任务队列节点
struct ChannelEvent
{
    // 类型  处理channel的方式
    int type;
    // channel
    struct Channel *channel;
    // 下一个节点
    struct ChannelEvent *_next;
};
struct Dispatcher;
struct EventLoop
{
    // 判断工作状态
    bool _isQuite;
    // 调度方法
    struct Dispatcher *_dispatcher;
    // 方法数据
    void *disepatherData;
    // 任务队列--头尾节点
    struct ChannelEvent *_head;
    struct ChannelEvent *_tail;
    // map
    struct ChannelMap *_channelmap;
    // 线程id 线程名
    pthread_t _threadID;
    char _threadName[32];
    // 互斥锁
    pthread_mutex_t _mutex;
    int socketpair[2];
};

// 初始化
// 主线程
struct EventLoop *initEventLoop();
// 子线程
struct EventLoop *initEventLoopEx();
// 启动反应堆模型
int RunEventLoop(struct EventLoop *EventLoop);
// 处理激活的fd
int ActivateEvent(struct EventLoop *EventLoop, int events, int fd);
// 添加任务到任务队列
int AddTaskEventLoop(struct EventLoop *EventLoop, struct Channel *channel, int type);
// 处理任务队列的任务
int processTaskEventLoop(struct EventLoop *EventLoop);
//处理dispatcher中的节点
int AddEventLoop(struct EventLoop *EventLoop, struct Channel *channel);
int DeleteEventLoop(struct EventLoop *EventLoop, struct Channel *channel);
int ModifyEventLoop(struct EventLoop *EventLoop, struct Channel *channel);
//释放channel
int destroyChannel(struct EventLoop *EventLoop, struct Channel *channel);
