#include "TcpConnection.h"

int processRead(void *arg)
{
    struct TcpConnection *conn = (struct TcpConnection *)arg;
    int count = socketReadBuffer(conn->readBuf,conn->channel->_fd);
    if(count>0)
    {
        //收到http请求 解析http请求
    }
    else
    {
        //断开连接
    }
}
// 初始化
struct TcpConnection *initTcpConnection(struct EventLoop *EventLoop, int fd)
{
    struct TcpConnection *conn = (struct TcpConnection *)malloc(sizeof(struct TcpConnection));
    conn->EventLoop = EventLoop;
    conn->readBuf = initBuffer(10240);
    conn->writeBuf = initBuffer(10240);
    sprintf(conn->name, "TcpConnection-%d", fd);
    conn->channel = initchannel(fd, readevent, NULL, processRead, conn);
    AddTaskEventLoop(EventLoop, conn->channel, ADD);
    return conn;
}