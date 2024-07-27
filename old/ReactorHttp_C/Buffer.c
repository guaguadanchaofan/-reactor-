#define _GNU_SOURCE
#include "Buffer.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/uio.h>
#include "Log.h"
// 初始化
struct Buffer *initBuffer(int size)
{
    struct Buffer *buffer = (struct Buffer *)malloc(sizeof(struct Buffer));
    
    buffer->_capacity = size;
    buffer->_readPos = buffer->_writePos = 0;
    buffer->_data = (char *)malloc(size);
    memset(buffer->_data, 0, size);
    return buffer;
}

// 销毁空间
void destroyBuffer(struct Buffer *buffer)
{
    if (buffer != NULL)
    {
        if (buffer->_data != NULL)
        {
            free(buffer->_data);
        }
    }
    free(buffer);
}

// 检测buffer中可写的内存大小
int bufferWriteAbleSize(struct Buffer *buffer)
{
    return buffer->_capacity - buffer->_writePos;
}
// 检测buffer中可读的内存大小
int bufferReadAbleSize(struct Buffer *buffer)
{
    return buffer->_writePos - buffer->_readPos;
}

// 扩容
void extendRoomBuffer(struct Buffer *buffer, int size)
{
    // 1.内存足够不需要扩容
    if (bufferWriteAbleSize(buffer) >= size)
    {
        return; // 不需要扩容
    }
    // 2.内存足够但是不连续
    else if (buffer->_readPos + bufferWriteAbleSize(buffer) >= size)
    {
        // 得到未读内存大小
        int readable = bufferReadAbleSize(buffer);
        // 移动内存
        memcpy(buffer->_data, buffer->_data + buffer->_readPos, readable);
        // 更新位置
        buffer->_readPos = 0;
        buffer->_writePos = readable;
    }
    // 3.内存不够需要扩容
    else
    {
        void *tmp = realloc(buffer->_data, buffer->_capacity + size);
        assert(tmp != NULL);
        memset(tmp + buffer->_capacity, 0, size);
        buffer->_data = tmp;
        buffer->_capacity = buffer->_capacity + size;
    }
}

int appendDataBuffer(struct Buffer *buffer, const char *data, int size)
{
    if (buffer == NULL || data == NULL || size <= 0)
    {
        return -1;
    }
    // 判断是否扩容
    extendRoomBuffer(buffer, size);
    // 数据拷贝
    memcpy(buffer->_data + buffer->_writePos, data, size);
    buffer->_writePos += size;
    return 0;
}
int appendStringBuffer(struct Buffer *buffer, const char *data)
{
    int size = strlen(data);
    int ret = appendDataBuffer(buffer, data, size);
    return ret;
}

// 接受套接字
int socketReadBuffer(struct Buffer *buffer, int fd)
{
    // read recv readv
    struct iovec vec[2];
    vec[0].iov_base = buffer->_data + buffer->_writePos;
    int writreable = bufferWriteAbleSize(buffer);
    vec[0].iov_len = writreable;
    void *tmp = malloc(40960);
    vec[1].iov_base = tmp;
    vec[1].iov_len = 40960;
    int result = readv(fd, vec, 2);
    if (result == -1)
    {
        return -1;
    }
    // 如果结果小于可写内存说明都写入到vec[0]
    else if (result <= writreable)
    {
        buffer->_writePos = buffer->_writePos + result;
    }
    // vec[1]中也有数据需要拷贝
    else
    {
        // 注意可写数据位置更改
        buffer->_writePos = buffer->_capacity;
        appendDataBuffer(buffer, tmp, result - writreable);
    }

    // 注意释放内存
    free(tmp);
    // 返回接收到多少个字节
    return result;
}

// 根据\r\n取出一行 找到起中间数据块得位置，返回改位置
char *findCRLFBuffer(struct Buffer *buffer)
{
    // strstr---从大字符串堆中找到子字符串（遇到\0结束）
    // memmem---从大数据堆中找到子数据（需要指定数据大小）
    char *ptr = memmem(buffer->_data + buffer->_readPos, bufferReadAbleSize(buffer), "\r\n", 2);
    return ptr;
}

// 发送数据
int sendDataBuffer(struct Buffer *buffer, int fd)
{
    // 判断有无数据
    int readable = bufferReadAbleSize(buffer);
    if (readable > 0)
    {
        int count = send(fd, buffer->_data + buffer->_readPos, readable, MSG_NOSIGNAL);
        if (count)
        {
            buffer->_readPos += count;
            usleep(10);
        }
        return count;
    }
    return 0;
}
