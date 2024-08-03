#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include <functional>
#include <memory>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <string>

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("[%s:%s:%d]TcpConnention EventLoop is nullptr!", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &nameArg,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peeraddr)
    : loop_(CheckLoopNotNull(loop)),
      name_(nameArg),
      socket_(new Socket(sockfd)),
      state_(kConnecting),
      reading_(true),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peeraddr),
      highWaterMark_(64 * 1024 * 1024)
{
    // channel相应的回调函数，poller给channel通知感兴趣的时间发生了，channel会调用相应的回调
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true); // 设置tcp保活机制
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnectio:dtor[%s] at fd = %d state = %d", name_.c_str(), channel_->fd(), (int)state_);
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading(); // 注册channel的读事件

    // 新连接建议，执行回调
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroy()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp reveiveTime)
{
    int savedErrono = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrono);
    if (n > 0)
    {
        // 已经建立连接的用户，有可读事件发生了，调用用户传入的回调操作onMessage
        messageCallback_(shared_from_this(), &inputBuffer_, reveiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrono;
        LOG_ERROR("TcpConnection::handleRead\n");
        handleError();
    }
}
void TcpConnection::handleWrite()
{

    if (channel_->isWriting())
    {
        int savedErrono = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrono);
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    // 唤醒loop对应的thread线程，执行回调
                    loop_->queueInloop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnectiong)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite\n");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd = %d is down , no more writing\n", channel_->fd());
    }
}

// poller -> channel::closeCallback -> TcpConnection::handleclose
void TcpConnection::handleClose()
{
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n", channel_->fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr); // 执行连接关闭的回调
    closeCallback_(connPtr);      // 关闭连接的回调  执行的是tcpserver::removeconnection回调方法
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("Connection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}

void TcpConnection::send(const std::string &buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            loop_->runInloop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

// 发送数据 应用写的快 ，内核发的慢 ，需要把待发送数据写入缓冲区，而且设置了水位回调
void TcpConnection::sendInLoop(const void *data, size_t len)
{
    ssize_t nwrote = 0;
    ssize_t remaining = len;
    bool faultError = false;
    // 之前调用过改Connection的shutdown，不能再发送了
    if (state_ == kDisconnected)
    {
        LOG_ERROR("disconnection, give up writing!\n");
        return;
    }
    // channel第一次开始写数据而且缓冲区没有待发送数据
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                // 既然在这里数据全部发送完成，就不用再给channel设置epollout事件了
                loop_->queueInloop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop\n");
                if (errno == EPIPE || errno == ECONNRESET) // sigpipe reset
                {
                    faultError = true;
                }
            }
        }
    }
    // 说明当前这一次write，并没有把数据全部发送数据，剩余的数据需要保存到缓冲区当中，然后给channel
    // 注册epollout事件，poller发现tcp发送缓冲区有空间，会通知响应的sock->channel,调用hanlewrite回调方法
    // 最终也就是调用tcpconnection：：handlewrite方法，把发送缓冲区中的数据全部发送完成
    if (!faultError && remaining > 0)
    {
        size_t oldlen = outputBuffer_.readableBytes(); // 剩余的待发送时数据的长度
        if (oldlen + remaining >= highWaterMark_ && oldlen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInloop(std::bind(highWaterMarkCallback_, shared_from_this(), oldlen + remaining));
        }
        outputBuffer_.append((char *)data + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting(); // 这里一定要注册channel的写事件否则poller不会给channel通知epollout
        }
    }
}
void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}
void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnectiong);
        loop_->runInloop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}