#pragma once
#include <stdbool.h>
#include "Buffer.h"
#include "HttpResponse.h"
// 定义请求头键值对
struct RequestHeader
{
    char *key;
    char *value;
};

// 当前解析状态
enum HttpRequestState
{
    ParseReqLine,
    ParseReqHeader,
    ParseReqBody,
    ParseReqDone,
};

// 定义http结构体
struct HttpRequest
{
    // 请求行
    char *method;
    char *url;
    char *version;
    // 请求头
    struct RequestHeader *reqHeader;
    int reqHeaderNum; // 请求头数量
    enum HttpRequestState curState;
};

// 重置
void resetHttpRequestEX(struct HttpRequest *reqheader);
void resetHttpRequest(struct HttpRequest *reqheader);
// 初始化
struct HttpRequest *initHttpRequest();
// 内存释放
void destroyHttpRequest(struct HttpRequest *reqheader);
// 获取请求状态
enum HttpRequestState HttpRequestState(struct HttpRequest *reqheader);
// 添加请求头
void addHttpRequestHeader(struct HttpRequest *reqheader, const char *key, const char *value);
// 根据key得到value值
char *getHttpRequestHeader(struct HttpRequest *reqheader, const char *key);
// 解析请求行
bool parseHttpRequestLine(struct HttpRequest *reqheader, struct Buffer *buffer);
// 解析请求头
bool parseHttpRequestHeader(struct HttpRequest *reqheader, struct Buffer *buffer);
// 解析完整Http请求
bool parseHttpRequest(struct HttpRequest *reqheader, struct Buffer *readbuffer, struct Buffer *writebuffer, struct HttpResponse *resp, int socket);
// 处理http请求
bool processHttpRequest(struct HttpRequest *reqheader, struct HttpResponse *resp);
// 获取文件类型
const char *getFileType(const char *name);
// 发送目录
void sendDir(const char *dirName, struct Buffer *sendbuf, int cfd);
// 发送文件
void sendFile(const char *namefile, struct Buffer *sendbuf, int cfd);
void decodeMsg(char *to, char *from);
