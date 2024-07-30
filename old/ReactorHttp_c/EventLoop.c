#include "EventLoop.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include<stdio.h>
#include"Log.h"
// 主线程
struct EventLoop *initEventLoop()
{
    return initEventLoopEx(NULL);
}

// 写数据
void taskWakeUp(struct EventLoop *EventLoop)
{
    const char *msg = "好难好难好难好累好累好累";
    write(EventLoop->socketpair[0], msg, sizeof(msg));
}
// 读数据
int readLocalMessage(void *arg)
{
    struct EventLoop *EventLoop = (struct EventLoop *)arg;
    char buf[256];
    read(EventLoop->socketpair[1], buf, sizeof(buf));
    return 0;
}
// 子线程
struct EventLoop *initEventLoopEx(const char *threaName)
{
    struct EventLoop *data = (struct EventLoop *)malloc(sizeof(struct EventLoop));
    data->_isQuite = false;
    // 指定使用的调度模型（Epolldispatch）
    data->_dispatcher = &Epolldispatch;
    data->_threadID = pthread_self();
    pthread_mutex_init(&data->_mutex, NULL);
    strcpy(data->_threadName, threaName == NULL ? "MainThread" : threaName);
    // 模型使用的数据库初始化
    data->disepatherData = data->_dispatcher->init();
    // 链表初始化
    data->_head = data->_tail = NULL;
    // map初始化
    data->_channelmap = initChannelMap(128);
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, data->socketpair);
    if (ret == -1)
    {
        perror("socketpair");
        exit(0);
    }
    // 规定：socketpair[0] 写数据  socketpair[1] 收数据
    struct Channel *channel = initchannel(data->socketpair[0], readevent, NULL, readLocalMessage,NULL, data);
    // channel添加到任务队列
    AddTaskEventLoop(data, channel, ADD);
    return data;
}
// 启动反应堆模型
int RunEventLoop(struct EventLoop *EventLoop)
{
    assert(EventLoop != NULL);
    // 取出事件分发和检测模型
    struct Dispatcher *dispatcher = EventLoop->_dispatcher;
    // 非正常情况
    if (EventLoop->_threadID != pthread_self())
    {
        return -1;
    }
    while (!EventLoop->_isQuite)
    {
        dispatcher->dispatch(EventLoop, 2); // 超时时长2s
        processTaskEventLoop(EventLoop);
    }
}
// 处理激活的fd
int ActivateEvent(struct EventLoop *EventLoop, int events, int fd)
{
    Debug("处理激活的fd");
    if (fd < 0 && EventLoop == NULL)
    {
        return -1;
    }
    // 取出对应文件描述符的channel
    struct Channel *channel = EventLoop->_channelmap->_list[fd];
    assert(channel->_fd == fd);
    // 判断事件类型
    if (events & readevent && channel->_readcallback)
    {
        // 调用对应的回调函数
        Debug("调用对应的回调函数_readcallback");
        channel->_readcallback(channel->_arg);
    }
    if (events & writevent && channel->_writecallback)
    {
        Debug("调用对应的回调函数_writecallback");
        channel->_writecallback(channel->_arg);
    }
    return 0;
}

// 添加任务到任务队列
int AddTaskEventLoop(struct EventLoop *EventLoop, struct Channel *channel, int type)
{
    // 加锁
    pthread_mutex_lock(&EventLoop->_mutex);
    // 创建新节点
    struct ChannelEvent *node = (struct ChannelEvent *)malloc(sizeof(struct ChannelEvent));
    node->type = type;
    node->channel = channel;
    if (EventLoop->_head == NULL)
    {
        EventLoop->_head = EventLoop->_tail = node;
        node->_next = NULL;
    }
    else
    {
        EventLoop->_tail->_next = node;
        EventLoop->_tail = node;
        node->_next = NULL;
    }
    pthread_mutex_unlock(&EventLoop->_mutex);
    // 处理节点
    // 细节：
    // 1.节点添加可能是当前线程也可能是其他线程（主线程）
    //  1).修改fd，当前子线程发起，当前子线程处理
    //  2).添加新的fd，由主线程添加
    // 2.不能让主线程处理任务队列，需要由当前的子线程去处理
    // 判断属于什么线程
    if (EventLoop->_threadID == pthread_self())
    {
        // 子线程
        processTaskEventLoop(EventLoop);
    }
    else
    {
        // 主线程--告诉子线程可以去处理任务队列里的任务
        // 1.子线程在工作 2.子线程被阻塞了：select poll epoll
        // 激活子线程
        taskWakeUp(EventLoop);
    }
}

// 处理任务队列的任务
int processTaskEventLoop(struct EventLoop *EventLoop)
{
    pthread_mutex_lock(&EventLoop->_mutex);
    // 取出头节点
    struct ChannelEvent *head = EventLoop->_head;
    while (head != NULL)
    {
        struct Channel *channel = head->channel;
        if (head->type == ADD)
        {
            // 添加
            AddEventLoop(EventLoop,channel);
        }
        if (head->type == DELETE)
        {
            // 删除
            DeleteEventLoop(EventLoop,channel);
        }
        if (head->type == MODIFY)
        {
            // 修改
            ModifyEventLoop(EventLoop,channel);
        }
        struct ChannelEvent *tmp = head;
        head = head->_next;
        free(tmp);
    }
    EventLoop->_head = EventLoop->_tail = NULL;
    pthread_mutex_unlock(&EventLoop->_mutex);
}

// 处理dispatcher中的节点
int AddEventLoop(struct EventLoop *EventLoop, struct Channel *channel)
{
    int fd = channel->_fd;
    struct ChannelMap *channelmap = EventLoop->_channelmap;
    if (fd >= channelmap->_size)
    {
        if (!ChannelMapAddROM(channelmap, fd, sizeof(struct ChannelMap *)))
        {
            return -1;
        };
    }
    // 找到对应fd的数组位置，并存储
    if (channelmap->_list[fd] == NULL)
    {
        channelmap->_list[fd] = channel;
        EventLoop->_dispatcher->add(channel, EventLoop);
    }
}
int DeleteEventLoop(struct EventLoop *EventLoop, struct Channel *channel)
{
    int fd = channel->_fd;
    struct ChannelMap *channelmap = EventLoop->_channelmap;
    if (fd >= channelmap->_size)
    {
        return -1;
    }
    int ret = EventLoop->_dispatcher->remove(channel, EventLoop);
    return ret;
}
int ModifyEventLoop(struct EventLoop *EventLoop, struct Channel *channel)
{
    int fd = channel->_fd;
    struct ChannelMap *channelmap = EventLoop->_channelmap;
    if (fd >= channelmap->_size||channelmap->_list[fd]==NULL)
    {
        return -1;
    }
    int ret = EventLoop->_dispatcher->modify(channel, EventLoop);
    return ret;
}


//释放channel
int destroyChannel(struct EventLoop *EventLoop, struct Channel *channel)
{
    struct ChannelMap* channelmap =EventLoop->_channelmap;
    //删除channel和fd对应的关系
    channelmap->_list[channel->_fd]=NULL;
    //关闭fd
    close(channel->_fd);
    //释放channel
    free(channel);
}
