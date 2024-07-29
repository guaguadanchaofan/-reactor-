#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>
#include <memory>
#include"Logger.h"
const int kNoneEvent = 0;
const int kReadEvent = EPOLLIN | EPOLLPRI;
const int kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : evloop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      tied_(false) {}
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}
// 改变channel所表示的fd的events事件后，update负责在poller里面更改fd响应的事件epollctl
void Channel::update()
{
    // 通过channel所属的loop，调用poller的方法
    // add code
    evloop_->updateChannel(this);
}
// 把当前的channel删掉
void Channel::remove()
{
    evloop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            handleEventWrithGuARD(receiveTime);
        }
    }
}
void Channel::handleEventWrithGuARD(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d",revents_);
    
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
        if (closeCallback_)
            closeCallback_();

    if (revents_ & EPOLLERR)
        if (errorCallback_)
            errorCallback_();

    if (revents_ & (EPOLLIN | EPOLLPRI))
        if (readCallback_)
            readCallback_(receiveTime);

    if (revents_ & EPOLLOUT)
        if (writeCallback_)
            writeCallback_();
}


Channel::~Channel() {}