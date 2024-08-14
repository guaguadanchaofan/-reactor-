#pragma once
#include <vector>
#include <stddef.h>
#include<string>
#include<algorithm>
class Buffer
{
public:
    static const size_t kChreapPrepend = 8;
    static const size_t kInitialSize = 1024;
    explicit Buffer(size_t initalSize = kInitialSize)
    : buffer_(kChreapPrepend + initalSize),
    readerIndex_(kChreapPrepend),
    wirteIndex_(kChreapPrepend)
    {}
    size_t readableBytes()const
    {
        return wirteIndex_ - readerIndex_;
    }
    size_t writeableBytes()const
    {
        return buffer_.size() - wirteIndex_;
    }
    size_t prependableBytes()const
    {
        return readerIndex_;
    } 

    //返回可读数据的起始地址
    const char* peek()const 
    {
        return begin() + readerIndex_;
    }

    void retrieve(size_t len) //检索
    {
        if(len < readableBytes())
        {
            readerIndex_ += len;  //应用只读取了一部分len长度
        }
        else //len == readableBytes
        {
            retrieveAll();
        }
    }
    void retrieveAll()
    {
        readerIndex_ = wirteIndex_ = kChreapPrepend;
    }

    //把onmessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsstring(readableBytes());
    }

    std::string retrieveAsstring(size_t len) //以字符串形式读取
    {
        std::string result(peek(),len); //从读地址处 读len长度大小
        retrieve(len);  //重置缓冲区大小
        return result;  //返回读取到的数据
    }

    void ensureWriteableBytes(size_t len) //确保缓冲区内存足够
    {
        if(writeableBytes() < len)  
        {
            makeSpace(len); //如果不够就扩容
        }
    }
    //把[data,data+len] 内存上的数据，添加到writeable缓冲区当中
    void append(const char* data , size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data,data+len,beginWrite());
        wirteIndex_ += len;
    }
    char* beginWrite()
    {
        return begin() + wirteIndex_;
    }
    const  char* beginWrite()const
    {
        return begin() + wirteIndex_;
    }
    //从fd上读取数据
    ssize_t readFd(int fd , int * saveErrnp);
    ssize_t writeFd(int fd , int * saveErrnp);
private:
    void makeSpace(size_t len) //扩容
    {
        if(writeableBytes() + prependableBytes() < len + kChreapPrepend) //如果可写大小 小于对应长度
        {
            buffer_.resize(wirteIndex_+len); //扩容
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + wirteIndex_,begin() + kChreapPrepend);
            readerIndex_ = kChreapPrepend;
            wirteIndex_ = readerIndex_ + readable;
        }
    }

    char* begin()
    {
        return &*buffer_.begin();
    }
    const char* begin()const
    {
        return &*buffer_.begin();
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t wirteIndex_;
};
