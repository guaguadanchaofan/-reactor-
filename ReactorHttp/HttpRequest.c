#define _GNU_SOURCE
#include "HttpRequest.h"
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "HttpResponse.h"
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

char *splitRequestLine(char *start, char *end, char *sub, char **ptr)
{
    char *space = end;
    if (sub = !NULL)
    {
        space = memmem(start, end - start, sub, strlen(sub));
        assert(space != NULL);
    }
    int lenght = space - start;
    char *tmp = (char *)malloc(lenght + 1);
    strncpy(tmp, start, lenght);
    tmp[lenght] = '\0';
    *ptr = tmp;
    return space + 1;
}

// 解析请求行
bool parseHttpRequestLine(struct HttpRequest *reqheader, struct Buffer *readbuf)
{
    // 读出请求行保存字符串结束地址
    char *end = findCRLFBuffer(readbuf);
    // 保存字符串起始地址
    char *start = readbuf->_data + readbuf->_readPos;
    // 计算请求行总长度
    int lineSize = end - start;
    if (lineSize)
    {
        start = splitRequestLine(start, end, " ", &reqheader->method);
        start = splitRequestLine(start, end, " ", &reqheader->url);
        splitRequestLine(start, end, NULL, &reqheader->version);
#if 0
        // get /xxx/xxx/xxx.txt http/1.1
        // 请求方式
        char *space = memmem(start, lineSize, " ", 1);
        assert(space != NULL);
        int methodSize = space - start;
        reqheader->method = (char *)malloc(methodSize + 1);
        strncpy(reqheader->method, start, methodSize);
        reqheader->method[methodSize] = '\0';

        // 请求静态资源
        start = methodSize + 1;
        space = memmem(start, end - start, " ", 1);
        assert(space != NULL);
        int urlSize = space - start;
        reqheader->url = (char *)malloc(urlSize + 1);
        strncpy(reqheader->url, start, urlSize);
        reqheader->url[urlSize] = '\0';
        // http 版本
        start = urlSize + 1;
        // int versionSize = space-start;
        reqheader->url = (char *)malloc(end - start + 1);
        strncpy(reqheader->url, start, end - start);
        reqheader->method[end - start] = '\0';
#endif
        // 为解析请求头做准备
        readbuf->_readPos += (lineSize + 2);
        reqheader->curState = ParseReqHeader;
    }
}

// 解析请求头
bool parseHttpRequestHeader(struct HttpRequest *reqheader, struct Buffer *readbuf)
{
    char *end = findCRLFBuffer(readbuf);
    if (end != NULL)
    {
        char *start = readbuf->_data + readbuf->_readPos;
        int lineSize = end - start;
        char *space = memmem(start, lineSize, ": ", 2);
        if (space != NULL)
        {
            // 拷贝key
            char *key = (char *)malloc(space - start + 1);
            strncpy(key, start, space - start);
            key[space - start] = '\0';
            // 拷贝value
            char *value = (char *)malloc(end - space - 2 + 1);
            strncpy(value, space + 2, end - space - 2);
            value[end - space - 2] = '\0';

            addHttpRequestHeader(reqheader, key, value);
            readbuf->_readPos += lineSize;
            readbuf->_readPos += 2;
            return true;
        }
        else
        {
            // 请求头被解析完了, 跳过空行
            readbuf->_readPos += 2;
            // 修改解析状态
            // 忽略 post 请求, 按照 get 请求处理
            reqheader->curState = ParseReqDone;
        }
    }
    return false;
}

// 解析完整Http请求
bool parseHttpRequest(struct HttpRequest *reqheader, struct Buffer *readbuffer, struct Buffer *writebuffer, struct HttpResponse *resp, int socket)
{
    bool flag = true;
    while (reqheader->curState != ParseReqDone)
    {
        switch (reqheader->curState)
        {
        case ParseReqLine:
            flag = parseHttpRequestLine(reqheader, readbuffer);
            break;
        case ParseReqHeader:
            flag = parseHttpRequestHeader(reqheader, readbuffer);
            break;
        case ParseReqBody:
            break;
        default:
            break;
        }
    }
    if (!flag)
    {
        return flag;
    }
    // 判断是否解析完了，如果完了需要准备恢复数据
    if (reqheader->curState == ParseReqDone)
    {
        // 根据解析数据做出对应处理
        processHttpRequest(reqheader, resp);
        // 组织响应数据发送到客户端
        prepareMsgHttpResponse(resp, writebuffer, socket);
    }
    reqheader->curState = ParseReqLine; // 状态还原
    return flag;
}

// 处理http请求
bool processHttpRequest(struct HttpRequest *reqheader, struct HttpResponse *resp)
{
    // 只解析get请求
    if (strcasecmp(reqheader->method, "get") != 0)
    {
        // 没找到返回-1
        return -1;
    }
    decodeMsg(reqheader->url, reqheader->url);

    // 处理客户端请求的静态资源（目录或文件）
    char *file = NULL;
    // 请求的资源是不是根目录
    if (strcmp(reqheader->url, "/") == 0)
    {
        file = "./";
    }
    else
    {
        file = reqheader->url + 1;
    }
    // 获取文件属性
    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1)
    {
        // 文件不存在--404
        // sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
        // sendFile("404.html", cfd);
        resp->StatusCode = 404;
        strcpy(resp->StatusMsg, "Not Found");
        strcpy(resp->fileName, "404.html");
        addHeaderHttpResponse(resp, "Content-type", getFileType("html"));
        resp->sendDataFunc = sendFile;
        return 0;
    }
    resp->StatusCode = 200;
    strcpy(resp->StatusMsg, "OK");
    strcpy(resp->fileName, file);
    // 判断文件类型
    if (S_ISDIR(st.st_mode))
    {
        // 是目录
        // sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
        // sendDir(cfd, file);
        addHeaderHttpResponse(resp, "Content-type", getFileType(file));
        resp->sendDataFunc = sendDir;
    }
    else
    {
        // 是文件
        // sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
        // sendFile(file, cfd);
        char tmp[12] = {0};
        sprintf(tmp, st.st_size);
        addHeaderHttpResponse(resp, "Content-type", getFileType(file));
        addHeaderHttpResponse(resp, "Content-length", tmp);
        resp->sendDataFunc = sendFile;
    }
}

// 将字符转换为整形数
int hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

// 解码
// to 存储解码之后的数据, 传出参数, from被解码的数据, 传入参数
void decodeMsg(char *to, char *from)
{
    for (; *from != '\0'; ++to, ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            *to = hexToDec(from[1]) * 16 + hexToDec(from[2]);

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            *to = *from;
        }
    }
    *to = '\0';
}
// 获取文件类型
const char *getFileType(const char *name)
{
    // a.jpg a.mp4 a.html
    // 自右向左查找‘.’字符, 如不存在返回NULL
    const char *dot = strrchr(name, '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8"; // 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

/*------------------------------------------------------------------------------------------*/

// 发送目录
int sendDir(const char *dirName, struct Buffer *sendbuf, int cfd)
{

    char buf[4096] = {0};
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName);
    struct dirent **namelise;
    int num = scandir(dirName, &namelise, NULL, alphasort);
    for (int i = 0; i < num; i++)
    {
        // 取出文件名 namelist 是一个指向 struct dirent* tmp[]的指针
        char *name = namelise[i]->d_name;
        struct stat st;
        char subPath[1024] = {0};
        sprintf(subPath, "%s/%s", dirName, name);
        stat(subPath, &st);
        // S_ISDIR 宏文件 判断是否是目录
        if (S_ISDIR(st.st_mode))
        {
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%1d</td></tr>", name, name, st.st_size);
        }
        else
        {
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%1d</td></tr>", name, name, st.st_size);
        }
        // send(cfd, buf, strlen(buf), 0);
        appendStringBuffer(sendbuf, buf);
#ifndef _SEND_MSG_AUTO
        sendDataBuffer(sendbuf, cfd);
#endif
        // 数据发送完缓冲区清零
        memset(buf, 0, sizeof(buf));
        free(namelise[i]);
    }
    sprintf(buf, "</table></body></html>");
    // send(cfd, buf, strlen(buf), 0);
    appendStringBuffer(sendbuf, buf);
#ifndef _SEND_MSG_AUTO
    sendDataBuffer(sendbuf, cfd);
#endif
    free(namelise);
    return 0;
}
/*------------------------------------------------------------------------------------------*/

// 发送文件
int sendFile(const char *namefile, struct Buffer *sendbuf, int cfd)
{
    // 打开文件
    int fd = open(namefile, O_RDONLY);
    assert(fd > 0);
#if 1
    // version 1
    while (1)
    {
        char buffer[1024];
        // 从对应文件描述符中读数据到buffer
        int len = read(fd, buffer, sizeof buffer);
        if (len > 0)
        {
            // 发送数据
            // send(cfd, buffer, sizeof buffer, 0);
            appendDataBuffer(sendbuf, buffer, len);
            sendDataBuffer(sendbuf, cfd);
            usleep(10); // 防止发送太快客户端来不及解析
        }
        // 没数据可读了
        else if (len == 0)
        {
            break;
        }
        else
        {
            close(cfd);
            perror("read");
        }
    }
#else
    // version 2
    // 第二个参数为文件偏移量
    off_t offset = 0;
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    while (offset < size)
    {
        // 第三个参数为文件偏移量
        sendfile(cfd, fd, &offset, size);
    }

#endif
    close(fd);
    return 0;
}