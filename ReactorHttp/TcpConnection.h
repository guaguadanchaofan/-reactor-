#pragma once
#include"EventLoop.h"
#include"Buffer.h"
#include"Channel.h"

struct TcpConnection
{   
    struct EventLoop* EventLoop;
    struct Buffer* readBuf;
    struct Buffer* writeBuf;
    struct Channel* channel;
    char name[32];
};


//初始化
struct TcpConnection* initTcpConnection(struct EventLoop* EventLoop,int fd);