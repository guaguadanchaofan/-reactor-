#pragma once
#include "noncopyable.h"
#include "EventLoop.h"
#include "EventLoopThreadPoll.h"
#include "Acceprot.h"
#include "InetAddress.h"
#include <functional>
#include <string>
#include <memory>
#include "Callbacks.h"
#include <atomic>
#include <unordered_map>

class TcpServer : noncopyable
{
public:
    enum Option
    {
        kNoReusePort,
        KreusePort
    };
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    TcpServer(EventLoop *loop, const InetAddress &listenAddr,std::string &nameArg, Option opt = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMassageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    // 设置底层subloop的数量
    void setThreadNum(int numThreads);
    // 开启服务器监听
    void start(); 

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    EventLoop *loop_; // baseloop

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceprot> acceptor_; // 运行在mainloop
    std::unique_ptr<EventLoopThreadPoll> threadPoll_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成的回掉
    ThreadInitCallback threadInitCallback_;       // 线程初始化

    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_;
};
