#pragma once
#include "noncopyable.h"
#include "Socket.h"
#include"Channel.h"
#include<functional>
class EventLoop;
class InetAddress;
class Acceprot : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd,const InetAddress&)>;
    Acceprot(EventLoop* loop, const InetAddress& listenaddr,bool reuseport);
    ~Acceprot();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {newConnectionCallback_ = cb;}
    bool listening()const{return listening_;}
    void listen();

private:
    void handleRead(); 
    EventLoop* loop_; //mainloop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};
