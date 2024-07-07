#pragma once
#include "Buffer.h"
#define _GNU_SOURCE
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
enum HttpStatusCode
{
    Unkonw,
    OK = 200,
    MovedPermanently = 301,
    MovedTemporarily = 302,
    BadRequest = 400,
    NotFound = 404
};

struct ResponseHeader
{
    char key[32];
    char value[128];
};

typedef void (*ResponseHandle)(const char *filename, struct Buffer *sendbuf, int socket);

struct HttpResponse
{
    // 状态行：状态码 状态描述
    enum HttpStatusCode StatusCode;
    char StatusMsg[128];
    // 响应头
    struct ResponseHeader *ResponseHeader;
    int headerNum;
    ResponseHandle sendDataFunc;
    char *fileName[128];
};

// 初始化
struct HttpResponse *initHttpResponse();
// 销毁
void destroyHttpResponse(struct HttpResponse *Resp);
// 添加请求头
void addHeaderHttpResponse(struct HttpResponse *Resp, const char *key, const char *value);
// 组织Http响应数据
void prepareMsgHttpResponse(struct HttpResponse *Resp, struct Buffer *sendbuf, int socket);