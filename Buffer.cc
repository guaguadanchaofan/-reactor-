#include "Buffer.h"
#include <sys/uio.h>
#include <unistd.h>

// 从fd上读取数据 Poller工作在LT模式
// buffer缓冲区是有大小的！ 但是从fd上读取数据的时候，却不知道TCP数据最终的大小
ssize_t Buffer::readFd(int fd, int *saveErrnp)
{
    char extrabuf[65536] = {0}; // 栈上的内存 64k

    struct iovec vec[2];

    const size_t writeable = writeableBytes(); // Buffer底层缓冲区剩余的可写空间大小
    
    vec[0].iov_base = begin() + wirteIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saveErrnp = errno;
    }
    else if (n <= writeable)  //buffer 的可写缓冲区已经狗存储读出来的数据了
    {
        wirteIndex_ += n;
    }
    else // extrabuf里面也写入了数据
    {
        wirteIndex_ = buffer_.size();
        append(extrabuf, n - writeable); //writeIndex_开始写 n-writeabke大小的数据

    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}