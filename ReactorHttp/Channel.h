#pragma once
#include<stdlib.h>
#include<stdbool.h>

//定义函数指针类型
typedef int(*HandelFunc)(void* arg);

//
enum FDevent
{
    writevent=0x01,
    readevent=0x02
};

//创建channel结构体
struct Channel
{
    //文件描述符
    int _fd;
    //事件
    int _events;
    //读写回调函数
    HandelFunc _writecallback;
    HandelFunc _readcallback;
    //回调函数参数
    void* _arg;
};



//channel初始化
struct Channel* initchannel(int fd,int event,HandelFunc writeFunc,HandelFunc readFunc,void* arg);


//修改fd的写事件（检测or不检测）
void writeEventEnable(struct Channel* Channel,bool flag);


//判断是否需要检测文件描述的写事件
bool iswriteEventEnable(struct Channel* Channel);
