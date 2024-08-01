#pragma once
#include "noncopyable.h"
#include <memory>
#include <string>
#include <atomic>
#include "InetAddress.h"
#include "Callbacks.h"
#include "Timestamp.h"
#include "Buffer.h"
class Channel;
class EventLoop;
class Socket;
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,
                  const std::string &name,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peeraddr);
    ~TcpConnection();
    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddress() const { return localAddr_; }
    const InetAddress &peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    // 关闭连接
    void shutdown();

    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }
    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback &cb)
    {
        messageCallback_ = cb;
    }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb, highWaterMark_ = highWaterMark;
    }

    // 连接建立
    void connectEstablished();
    // 连接销毁
    void connectDestroy();

    

    void send(const std::string& buf);
    void sendInLoop(const void* message,size_t len);
private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnectiong
    };
    void handleRead(Timestamp reveiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void setState(StateE stat);
    void shutdownInLoop();

    EventLoop *loop_; // 不是baseloop 因为都在tcpconnection都在subloop
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成的回掉
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;

    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};
