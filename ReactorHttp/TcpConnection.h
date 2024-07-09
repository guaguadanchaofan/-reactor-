#pragma once
#include"EventLoop.h"
#include"Buffer.h"
#include"Channel.h"
#include"HttpRequest.h"
#include"HttpResponse.h"

#define _SEND_MSG_AUTO
struct TcpConnection
{   
    struct EventLoop* EventLoop;
    struct Buffer* readBuf;
    struct Buffer* writeBuf;
    struct Channel* channel;
    char name[32];
    //协议
    struct HttpRequest* req;
    struct HttpResponse* resp;
};


//初始化
struct TcpConnection* initTcpConnection(struct EventLoop* EventLoop,int fd);
//释放资源
int detroyTcpConnection(void* arg); 