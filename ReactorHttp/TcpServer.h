#pragma once
#include "EventLoop.h"

struct Listener
{
    unsigned short port;
    int lfd;
};

struct TcpServer
{
    struct EventLoop *_mainloop;
    struct ThreadPool *_pool;
    struct Listener *_listener;
    int _threadNum;
};

// 初始化tcpserver
struct TcpServer *initTcpServer(unsigned short port, int threadNum);
// 初始化listener
struct Listener *initListener(unsigned short port);
// 启动tcpserver
void runTcpServer(struct TcpServer *tcpserver);