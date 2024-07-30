#pragma once
#include "EventLoop.h"
#include <stdbool.h>
#include "WorkerThread.h"

struct ThreadPool
{
    // 运行状态
    bool isStart;
    // 主反应堆
    struct EventLoop *_mainLoop;
    // 线程池大小
    int threadNum;
    // 下表
    int index;
    // 工作线程
    struct WorkerThread *workthreads;
};

// 初始化线程池
struct ThreadPool *initThreadPool(struct EventLoop *mainLoop, int count);
// 启动线程池
void runThreadPool(struct ThreadPool *pool);
// 取出线程池中某个子线程的反应堆实例
struct EventLoop *takeWorkEventLoop(struct ThreadPool *pool);