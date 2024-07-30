#pragma once
#include <pthread.h>
#include "EventLoop.h"
struct WorkerThread
{
    pthread_t _threadID; // ID
    char _name[24];
    struct EventLoop *_EventLoop; // 反应堆
    pthread_mutex_t _mutex;       // 互斥锁
    pthread_cond_t _cond;         // 条件变量
};

// 初始化 (外面申请内存传入，第几个线程)
int initWorkThread(struct WorkerThread *thread, int index);
//启动线程
void runWorkThread(struct WorkerThread *thread);