#pragma once

// 初始化监听套接字
int initListenFD(unsigned short port);

// 启动epoll
int epollRun(int lfd);

// 和客户端建立连接
// int acceptClient(int lfd, int epfd);
void *acceptClient(void *arg);

// 接http请求
// int recvHttpRequest(int cfd, int epfd);
void *recvHttpRequest(void *arg);

// 解析请求行
int parseRequestLine(const char *line, int cfd);

// 发送文件
int sendFile(const char *namefile, int cfd);

// 发送响应头（状态行+响应头）
int sendHeadMsg(int cfd, int stat, char *descr, const char *type, int length);

// 获取文件类型
const char *getFileType(const char *name);

// 发送目录
int sendDir(int cfd, const char *dirName);

// 解码中文
void decodeMsg(char *to, char *from);
