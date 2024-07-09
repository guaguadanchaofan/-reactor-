#pragma once
#include <sys/types.h>
#include <sys/socket.h>

struct Buffer
{
    // 指向内存数据块的指针
    char *_data;
    int _capacity;
    int _readPos;
    int _writePos;
};

// 初始化
struct Buffer *initBuffer(int size);
// 销毁空间
void destroyBuffer(struct Buffer *buffer);
// 扩容
void extendRoomBuffer(struct Buffer *buffer, int size);
// 检测buffer中可写的内存大小
int bufferWriteAbleSize(struct Buffer *buffer);
// 检测buffer中可读的内存大小
int bufferReadAbleSize(struct Buffer *buffer);
// 写内存 1.直接写 2.接受套接字数据
int appendDataBuffer(struct Buffer *buffer, const char *data, int size);
int appendStringBuffer(struct Buffer *buffer, const char *data);
// 接受套接字
int socketReadBuffer(struct Buffer *buffer, int fd);
// 根据\r\n取出一行 找到起中间数据块得位置，返回改位置
char *findCRLFBuffer(struct Buffer *buffer);
// 发送数据
int sendDataBuffer(struct Buffer *buffer, int fd);
