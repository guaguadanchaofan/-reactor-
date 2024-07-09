#include "HttpResponse.h"
#include "Log.h"
#define Header 16

// 初始化
struct HttpResponse *initHttpResponse()
{
    struct HttpResponse *resp = (struct HttpResponse *)malloc(sizeof(struct HttpResponse));
    resp->headerNum = 0;
    int size = sizeof(struct ResponseHeader) * Header;
    resp->ResponseHeader = (struct ResponseHeader *)malloc(size);
    resp->StatusCode = Unkonw;
    // 初始化数组
    bzero(resp->ResponseHeader, size);
    bzero(resp->StatusMsg, sizeof(resp->StatusMsg));
    //
    Debug("初始化HttpResponse.....");
    resp->sendDataFunc = NULL;
    return resp;
}

// 销毁
void destroyHttpResponse(struct HttpResponse *Resp)
{
    if (Resp != NULL)
    {
        if (Resp->ResponseHeader != NULL)
        {
            free(Resp->ResponseHeader);
        }
        free(Resp);
    }
}

// 添加请求头
void addHeaderHttpResponse(struct HttpResponse *Resp, const char *key, const char *value)
{
    Debug("添加请求头.....");
    if (Resp != NULL || key != NULL || value != NULL)
    {
        strcpy(Resp->ResponseHeader[Resp->headerNum].key, key);
        strcpy(Resp->ResponseHeader[Resp->headerNum].value, value);
        Resp->headerNum++;
    }
    return;
}

// 组织Http响应数据
void prepareMsgHttpResponse(struct HttpResponse *Resp, struct Buffer *sendbuf, int socket)
{
    Debug("组织Http响应数据.....");
    // 响应头
    char tmp[1024]={0};
    sprintf(tmp, "HTTP/1.1 %d %s\r\n", Resp->StatusCode, Resp->StatusMsg);
    appendStringBuffer(sendbuf, tmp);
    // 响应行
    for (int i = 0; i < Resp->headerNum; i++)
    {
        sprintf(tmp, "%s: %s\r\n", Resp->ResponseHeader[i].key, Resp->ResponseHeader[i].value);
        appendStringBuffer(sendbuf, tmp);
    }
    // 空行
    appendStringBuffer(sendbuf, "\r\n");
#ifndef MSG_SEND_AUTO
    sendDataBuffer(sendbuf, socket);
#endif
    // 回复响应数据
    Resp->sendDataFunc(Resp->fileName, sendbuf, socket);
}
