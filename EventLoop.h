#pragma once
#include "noncopyable.h"
#include <functional>
#include "Channel.h"
#include <vector>
#include <atomic>
#include "Timestamp.h"
#include <memory>
#include <mutex>
#include "CurrentThread.h"
class Channel;
class Poller;
class Timstamp;

class EventLoop
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();
    void loop(); //开启事件循环
    void quit(); //退出事件循环
    Timestamp pollReaturnTime()const{return pollReturnTime_;}
    void runInloop(Functor cb);//在当前loop中执行
    void queueInloop(Functor cb); //把cb放入队列中，唤醒loop所在的线程执行cb
    void wakeup(); //唤醒loop所在的线程

    //eventloop->poller
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
    
    //判断是否实在自己的线程里面
    bool isInLoopThread()const{return threadId_ == CurrentThread::tid();}
private:
    void handleRead();  //唤醒线程
    void doPendingFunctors(); //执行回调


    using ChannelList = std::vector<Channel *>; 
    std::atomic_bool looping_;                // 原子操作 ， 通过cas实现
    std::atomic_bool quit_;                   // 标志退出loop循环
    const pid_t threadId_;
    Timestamp pollReturnTime_; // poller返回发生事件channels的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;   //mainloop获取新用户轮询算法选择一个子反应堆 通过该成员唤醒子反应堆
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_; // 标识当前loop是否又需要执行的回调操作
    std::vector<Functor> pendingFuntors_;  //存储回调操作

    std::mutex mutex_;   //互斥锁用来保护vector容器线程安全
};
