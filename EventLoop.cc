#include "EventLoop.h"
#include "Logger.h"
#include "EpollPoller.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include "Poller.h"
#include "Channel.h"

// 防止一个线程创建多个eventloop  __thread->thread_local
__thread EventLoop *t_loopInThisThread = nullptr;
// poller超时时间 10s
const int kPollerTimeMs = 10000;

int creatEventfd() // 创建唤醒子线程的eventsfd
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("events error:%d\n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(creatEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
{
    //LOG_DEBUG("EventLoop created %p int thread %d\n", this, threadId_);
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists int this thread %d\n", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个eventloop都将监听wakeipchannel的epollin
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() // 开启事件循环
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping\n", this);
    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollerTimeMs, &activeChannels_);
        for (Channel *channel : activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping\n", this);
    looping_ = false;
}
void EventLoop::quit() // 退出事件循环
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInloop(Functor cb) // 在当前loop中执行
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInloop(cb);
    }
}
void EventLoop::queueInloop(Functor cb) // 把cb放入队列中，唤醒loop所在的线程执行cb
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFuntors_.emplace_back(cb);
    }
    // 唤醒响应的，需要执行上面回调操作的loop线程
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup(); // 唤醒loop所在线程 
    }
}

void EventLoop::wakeup() // 唤醒loop所在的线程
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_,&one,sizeof(one));
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8\n",n);
    }
}

// eventloop->poller
void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}
void EventLoop::doPendingFunctors() // 执行回调
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFuntors_);
    }
        for(const Functor& functor : functors)
        {
            functor();
        }
        callingPendingFunctors_ = false;
}
void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleread() reads %ld bytes instead of 8", n);
    }
}