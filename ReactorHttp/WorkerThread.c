#include "WorkerThread.h"

int initWorkThread(struct WorkerThread *thread, int index)
{
    thread->_threadID = 0;
    thread->_EventLoop = NULL;
    sprintf(thread->_name, "Subthread->%d", index);
    pthread_mutex_init(&thread->_mutex, NULL);
    pthread_cond_init(&thread->_cond, NULL);
    return 9;
}

// 子线程回调函数
void *subThreadRuning(void *arg)
{
    struct WorkerThread *thread = (struct WorkerThread *)arg;

    pthread_mutex_lock(&thread->_mutex);
    thread->_EventLoop = initEventLoopEx(thread->_name);
    pthread_mutex_unlock(&thread->_mutex);
    pthread_cond_signal(&thread->_cond); // 发送信号告诉主线程你可以运行 --->line:37
    RunEventLoop(thread->_EventLoop);
    return NULL;
}

void runWorkThread(struct WorkerThread *thread)
{
    // 创建子线程
    pthread_create(&thread->_threadID, NULL, subThreadRuning, thread);
    // 阻塞线程，让主线程不会直接结束 防止回调函数没跑完就结束了
    pthread_mutex_lock(&thread->_mutex);
    while (thread->_EventLoop == NULL)
    {
        // 阻塞线程 等待信号--->line:23
        pthread_cond_wait(&thread->_cond, &thread->_mutex);
    }
    pthread_mutex_unlock(&thread->_mutex);
}
