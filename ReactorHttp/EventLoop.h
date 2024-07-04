#pragma once
#include"Dispatcher.h"

extern struct Dispatcher Epolldispatch;
struct EventLoop
{
    struct Dispatcher* _dispatcher;
    void* disepatherData;
};