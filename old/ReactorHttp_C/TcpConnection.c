#include "TcpConnection.h"
#include <stdlib.h>
#include <stdio.h>
#include "Log.h"
int processRead(void *arg)
{
    struct TcpConnection *conn = (struct TcpConnection *)arg;
    int count = socketReadBuffer(conn->readBuf, conn->channel->_fd);
    Debug("接收到的http请求的数据：%s", conn->readBuf->_data + conn->readBuf->_readPos);
    if (count > 0)
    {
#ifdef _SEND_MSG_AUTO
        // 收到http请求 解析http请求
        writeEventEnable(conn->channel, true);
        // 修改dispatcher检测的集合--添加任务节点
        AddTaskEventLoop(conn->EventLoop, conn->channel, MODIFY);
#endif
        bool flag = parseHttpRequest(conn->req, conn->readBuf, conn->writeBuf, conn->resp, conn->channel->_fd);
        if (!flag)
        {
            // 解析失败
            char *msg = "HTTP/1.1 400 Bad Request\r\n";
            appendStringBuffer(conn->writeBuf, msg);
        }
    }
    else
    {
#ifdef _SEND_MSG_AUTO
        // 断开连接
        AddTaskEventLoop(conn->EventLoop, conn->channel, DELETE);
#endif
        return 0;
    }
#ifndef _SEND_MSG_AUTO
        // 断开连接
        AddTaskEventLoop(conn->EventLoop, conn->channel, DELETE);
#endif
}

int processwrite(void *arg)
{
    Debug("开始发送数据，基于写事件.....");
    struct TcpConnection *conn = (struct TcpConnection *)arg;
    int count = sendDataBuffer(conn->writeBuf, conn->channel->_fd);
    if (count > 0)
    {
        // 判断是否全部发出去了
        if (bufferReadAbleSize(conn->writeBuf) == 0)
        {
            // 不再检测写事件 --修改channel的事件
            writeEventEnable(conn->channel, false);
            // 修改dispatcher检测的集合--添加任务节点
            AddTaskEventLoop(conn->EventLoop, conn->channel, MODIFY);
            // 删除节点
            AddTaskEventLoop(conn->EventLoop, conn->channel, DELETE);
        }
    }
    return 0;
}
// 初始化
struct TcpConnection *initTcpConnection(struct EventLoop *EventLoop, int fd)
{
    Debug("initTcpConnection ");
    struct TcpConnection *conn = (struct TcpConnection *)malloc(sizeof(struct TcpConnection));
    conn->EventLoop = EventLoop;
    conn->readBuf = initBuffer(10240);
    conn->writeBuf = initBuffer(10240);
    conn->req = initHttpRequest();
    conn->resp = initHttpResponse();
    sprintf(conn->name, "TcpConnection-%d", fd);
    conn->channel = initchannel(fd, readevent, processwrite, processRead, detroyTcpConnection, conn);
    AddTaskEventLoop(EventLoop, conn->channel, ADD);
    Debug("和客户端建立连接 threadid:%d threadname:%s connname:%s ", EventLoop->_threadID, EventLoop->_threadName, conn->name);
    return conn;
}

// 释放资源
int detroyTcpConnection(void *arg)
{
    struct TcpConnection *conn = (struct TcpConnection *)arg;
    if (conn != NULL)
    {
        if (conn->readBuf && conn->writeBuf && bufferReadAbleSize(conn->readBuf) == 0 && bufferReadAbleSize(conn->writeBuf) == 0)
        {
            destroyBuffer(conn->readBuf);
            destroyBuffer(conn->writeBuf);
            destroyHttpRequest(conn->req);
            destroyHttpResponse(conn->resp);
            destroyChannel(conn->EventLoop, conn->channel);
        }
        free(conn);
    }
    Debug("连接断开，释放资源  connname: %s", conn->name);
    return 0;
}