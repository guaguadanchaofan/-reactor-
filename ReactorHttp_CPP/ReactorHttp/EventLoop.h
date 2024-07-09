#pragma once
#include "Dispatcher.h"
#include <thread>
#include <queue>
#include <map>
#include <mutex>
// 处理channel的方式
enum class ElemType : char
{
    ADD,
    DELETE,
    MODIFY
};

// 任务队列节点
struct ChannelEvent
{
    // 类型  处理channel的方式
    ElemType _type;
    // channel
    Channel *_channel;
    // 下一个节点
};

class Dispatcher;
class EventLoop
{
public:
    EventLoop()
    {
    }
    EventLoop(const string threaName)
    {
    }
    ~EventLoop()
    {
    }
    // 启动反应堆模型
    int Run();
    // 处理激活的fd
    int ActivateEvent(int events, ElemType fd);
    // 添加任务到任务队列
    int AddTask(Channel *channel, ElemType type);
    // 处理任务队列的任务
    int processTask();

    // 处理dispatcher中的节点
    int Add(Channel *channel);
    int Delete(Channel *channel);
    int Modify( Channel *channel);
    // 释放channel
    int freeChannel(Channel *channel);

private:
    // 判断工作状态
    bool _isQuite;
    // 该指针指向子类的实例
    Dispatcher *_dispatcher;
    // 任务队列--头尾节点
    queue<ChannelEvent *> _TaskQ;
    map<int, Channel *> _channelMap;
    // 线程id 线程名
    thread::id _threadID;
    string _threadName;
    // 互斥锁
    mutex _mutex;
    int _socketpair[2];
};
