#pragma once
// 定义函数指针类型
// typedef int (*HandelFunc)(void *arg);
using HandelFunc = int (*)(void *arg);

//
enum class FDevent
{
    TimeOut = 0x01,
    writevent = 0x02,
    readevent = 0x04
};

// 创建channel结构体
class Channel
{
public:
    Channel(int fd, ElemType event, HandelFunc writeFunc, HandelFunc readFunc, HandelFunc destroyFunc, void *arg);
    void writeEventEnable(bool flag);
    bool iswriteEventEnable();
    HandelFunc _writecallback;
    HandelFunc _readcallback;
    HandelFunc _destroycallback;

    inline int getfd()
    {
        return _fd;
    }
    inline int getevents()
    {
        return _events;
    }
    inline void *getarg()
    {
        return _arg;
    }

private:
    int _fd;
    int _events;
    void *_arg;
};
