#include "ThreadPool.h"
#include <assert.h>
// 初始化线程池
struct ThreadPool *initThreadPool(struct EventLoop *mainLoop, int count)
{
    struct ThreadPool *pool = (struct ThreadPool *)malloc(sizeof(struct ThreadPool));
    pool->isStart = false;
    pool->_mainLoop = mainLoop;
    pool->index = 0;
    pool->threadNum = count;
    pool->workthreads = (struct WorkerThread *)malloc(sizeof(struct WorkerThread) * count);
    return pool;
}

// 启动
void runThreadPool(struct ThreadPool *pool)
{
    assert(pool && !pool->isStart);
    // 判断是否由主线程启动
    if (pool->_mainLoop->_threadID != pthread_self())
    {
        // 不是主线程退出
        exit(0);
    }
    pool->isStart = true;
    // 判断线程池里面线程数量
    if (pool->threadNum > 0)
    {
        for (int i = 0; i < pool->threadNum; i++)
        {
            // 初始化线程
            initWorkThread(&pool->workthreads[i], i);
            // 启动线程
            runWorkThread(&pool->workthreads[i]);
        }
    }
}

// 取出线程池中某个子线程的反应堆实例
struct EventLoop *takeWorkEventLoop(struct ThreadPool *pool)
{
    assert(pool->isStart);
    // 判断是否由主线程启动
    if (pool->_mainLoop->_threadID != pthread_self())
    {
        // 不是主线程退出
        exit(0);
    }
    // 从线程池中找到一个子线程，然后取出里面的反应堆实例
    struct EVentLoop *EventLoop = pool->_mainLoop;
    if (pool->threadNum > 0)
    {
        EventLoop = pool->workthreads[pool->index]._EventLoop;
        pool->index = ++pool->index % pool->threadNum;
    }
    return EventLoop;
}
