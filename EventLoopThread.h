#pragma once
#include<functional>
#include<mutex>
#include<condition_variable>
#include"Thread.h"
#include <string>
class EventLoop;
class EventLoopThread : noncopyable
{
public:
using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb,const std::string &name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();
    EventLoop* loop_;
    bool exiting_;
    std::mutex mutex_;
    Thread thread_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

