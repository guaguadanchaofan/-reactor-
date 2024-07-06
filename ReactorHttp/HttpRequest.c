#define _GNU_SOURCE
#include "HttpRequest.h"
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include "Buffer.h"
#define HeaderSize 12

// 初始化
struct HttpRequest *initHttpRequest()
{
    struct HttpRequest *reqheader = (struct HttpRequest *)malloc(sizeof(struct HttpRequest));
    resetHttpRequestEX(reqheader);
    reqheader->reqHeader = (struct RequestHeader *)malloc(sizeof(struct RequestHeader) * HeaderSize);
    return reqheader;
}

// 重置
void resetHttpRequest(struct HttpRequest *reqheader)
{
    reqheader->curState = ParseReqLine;
    reqheader->method = NULL;
    reqheader->url = NULL;
    reqheader->version = NULL;
    reqheader->reqHeaderNum = 0;
}

void resetHttpRequestEx(struct HttpRequest *reqheader)
{
    free(reqheader->method);
    free(reqheader->url);
    free(reqheader->version);
    if (reqheader->reqHeaderNum != 0)
    {
        for (int i = 0; i < reqheader->reqHeaderNum; i++)
        {
            free(reqheader->reqHeader[i].key);
            free(reqheader->reqHeader[i].value);
        }
    }
    resetHttpRequest(reqheader);
}

// 内存释放
void destroyHttpRequest(struct HttpRequest *reqheader)
{
    if (reqheader != NULL)
    {
        resetHttpRequestEX(reqheader);
        free(reqheader);
    }
}

// 获取请求状态
enum HttpRequestState HttpRequestState(struct HttpRequest *reqheader)
{
    return reqheader->curState;
}

// 添加请求头
void addHttpRequestHeader(struct HttpRequest *reqheader, const char *key, const char *value)
{
    reqheader->reqHeader[reqheader->reqHeaderNum].key = key;
    reqheader->reqHeader[reqheader->reqHeaderNum].value = value;
    //++请求头数量
    reqheader->reqHeaderNum++;
}

// 根据key得到value值
char *getHttpRequestHeader(struct HttpRequest *reqheader, const char *key)
{
    if (reqheader != NULL)
    {
        for (int i = 0; i < reqheader->reqHeaderNum; i++)
        {
            if (strncasecmp(reqheader->reqHeader[i].key, key, strlen(key)))
            {
                return reqheader->reqHeader[i].value;
            }
        }
    }
    return NULL;
}

// 解析请求行
bool parseHttpRequest(struct HttpRequest *reqheader, struct Buffer *readbuf)
{
    // 读出请求行保存字符串结束地址
    char *end = findCRLFBuffer(readbuf);
    // 保存字符串起始地址
    char *start = readbuf->_data + readbuf->_readPos;
    // 计算请求行总长度
    int lineSize = end - start;
    if (lineSize)
    {
        // get /xxx/xxx/xxx.txt http/1.1
        // 请求方式
        char *space = memmem(start, lineSize," ",1);
        assert(space!=NULL);
        int methodSize = space-start;
        reqheader->method=(char*)malloc(methodSize+1);
        strncpy(reqheader->method,start,methodSize);
        reqheader->method[methodSize]='\0';
        
        //请求静态资源
        start=methodSize+1;
        space = memmem(start, end-start," ",1);
        assert(space!=NULL);
        int urlSize = space-start;
        reqheader->url=(char*)malloc(urlSize+1);
        strncpy(reqheader->url,start,urlSize);
        reqheader->url[urlSize]='\0';
        //http 版本
        start=urlSize+1;
        //int versionSize = space-start;
        reqheader->url=(char*)malloc(end-start+1);
        strncpy(reqheader->url,start,end-start);
        reqheader->method[end-start]='\0';

        //为解析请求头做准备
        readbuf->_readPos+=(lineSize+2);
        reqheader->curState=ParseReqHeader;
    }
}