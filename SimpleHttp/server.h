#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include<fcntl.h>
#include<unistd.h>
#include <stdio.h>

// 初始化监听套接字
int initListenFD(uint16_t port);

// 启动epoll
int epollRun(int lfd);

// 和客户端建立连接
int acceptClient(int lfd, int epfd);