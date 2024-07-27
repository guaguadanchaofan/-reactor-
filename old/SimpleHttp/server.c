#define _GNU_SOURCE
#include "server.h"
#include <dirent.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/sendfile.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

struct FdInfo
{
    int _fd;
    int _epfd;
    pthread_t tid;
};
// 初始化监听
int initListenFD(unsigned short port)
{
    // 1.创建监听的FD
    // 第一个参数为协议家族
    // AF_LOCAL 本地通信unix(7)
    // AF_INET  IPv4 Internet协议
    // AF_INET6 IPv6 Internet协议
    // 第二个参数指定套接字基于流式协议 还是数据报文协议
    /*SOCK_STREAM提供了有序的、可靠的、双向的、基于连接的字节流。
    SOCK_DGRAM支持数据报(无连接、不可靠、最大长度固定的消息)。*/
    // 第三个参数写0 代表流式协议的TCP  UDP同理
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        return -1;
    }

    // 2.设置端口复用（当服务器被关闭需要2msl==1min时间才能再次使用这个端口）
    /*int level: 选项定义的层次；目前仅支持SOL_SOCKET和IPPROTO_TCP层次
      int optname: 需设置的选项 SO_REUSEADDR（打开或关闭地址复用）
      const void *optval: 指针，指向存放选项值的缓冲区
      socklen_t optlen: optval缓冲区的长度*/
    int opt = 1;
    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1)
    {
        perror("setsockopt");
        return -1;
    }

    // 3.绑定端口号和IP地址
    //  创建struct sockaddr_in
    /*sin_family：地址族，通常设置为AF_INET表示IPv4协议。
      sin_port：端口号，以网络字节序表示。
      sin_addr：IP地址，以网络字节序表示。
      sin_zero：填充字段，通常设置为0。*/
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    // 端口用的网络字节序
    // htons()：用于将主机字节序的16位整数值转换为网络字节序。
    // ntohs()：用于将网络字节序的16位整数值转换回主机字节序。
    // htonl()：用于将主机字节序的32位整数值转换为网络字节序。
    // ntohl()：用于将网络字节序的32位整数值转换回主机字节序。
    addr.sin_port = htons(port);
    // 注意要采用大端（低地址储存高位） INADDR_ANY表示0不区分大小端INADDR_ANY表示可以用任何地址
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind");
        return -1;
    }

    // 4.设置监听
    // 第二个参数为backlog 表示待连接队列的最大长度 最大为128
    ret = listen(lfd, 128);
    if (ret == -1)
    {
        perror("listen");
        return -1;
    }

    // 5.返回值
    return lfd;
}

/*-----------------------------------------------------------------------------------------*/

// 启动epoll
int epollRun(int lfd)
{
    // 1.创建epoll实例（底层红黑树）
    // 参数size可忽略（liunx2.6.8）
    int epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll_create");
        return -1;
    }

    // 2.将监听套接字加载到epoll中（lfd上树）
    // 创建epoll事件结构体
    // EPOLLIN(读) EPOLLOUT(写)
    struct epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if (ret == -1)
    {
        perror("epoll_ctl");
        return -1;
    }

    // 3.检测（时间未知需要while循环检测）
    // 数组多大无所谓
    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);
    while (1)
    {
        // 事件等待，将就绪的事件数返回
        int num = epoll_wait(epfd, evs, size, -1);
        // 遍历就绪的事件
        for (int i = 0; i < num; i++)
        {
            struct FdInfo *info = (struct FdInfo *)malloc(sizeof(struct FdInfo));
            assert(info != NULL);
            int fd = evs[i].data.fd;
            info->_fd = fd;
            info->_epfd = epfd;
            // 如果当前fd是用于监听的文件描述符
            if (fd == lfd)
            {
                // 建立新连接（事件已经发生 不会阻塞）
                pthread_create(&info->tid, NULL, acceptClient, info);
            }
            // 用于通信
            else
            {
                // 主要是接受对端数据（读数据）
                pthread_create(&info->tid, NULL, recvHttpRequest, info);
            }
        }
    }
}

/*-----------------------------------------------------------------------------------------*/

// 和客户端建立连接
// int acceptClient(int lfd, int epfd)
void* acceptClient(void *arg)
{
    struct FdInfo *info = (struct FdInfo *)arg;
    // 1.建立连接
    // 第23个参数为传出参数用于保存客户端的ip跟port信息
    int cfd = accept(info->_fd, NULL, NULL);
    if (cfd == -1)
    {
        perror("accept");
        return NULL;
    }

    // 2.设置非阻塞
    /*复制一个现有的描述符(cmd=F_DUPFD)
    获得/设置文件描述符标记(cmd=F_GETFD或F_SETFD)
    获得/设置文件状态标记(cmd=F_GETFL或F_SETFL)
    获得/设置异步I/O所有权(cmd=F_GETOWN或F_SETOWN)
    获得/设置记录锁(cmd=F_GETLK,F_SETLK或F_SETLKW)*/
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    // ET(边缘触发 需要设置非阻塞) LT(水平触发)
    fcntl(cfd, F_SETFL, flag);

    // 3.cfd添加到epoll中
    // 创建epoll事件结构体
    struct epoll_event ev;
    ev.data.fd = cfd;
    // 检测读事件就绪(读事件|边缘触发事件)
    ev.events = EPOLLIN | EPOLLET;
    int ret = epoll_ctl(info->_epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1)
    {
        perror("epoll_ctl");
        return NULL;
    }
    printf("acceptClient tid:%ld\n", info->tid);
    free(info);
}

/*------------------------------------------------------------------------------------------*/

// 接http请求
// int recvHttpRequest(int cfd, int epfd)
void *recvHttpRequest(void *arg)
{
    struct FdInfo *info = (struct FdInfo *)arg;
    int len = 0, totle = 0;
    char tmp[1024] = {0};
    char buffer[4096] = {0};
    while ((len = recv(info->_fd, tmp, sizeof tmp, 0)) > 0)
    {
        if (totle + len < sizeof buffer)
        {
            memcpy(buffer + totle, tmp, len);
        }
    }
    // 判断是否接受完毕
    if (len == -1 && errno == EAGAIN)
    {
        // 解析请求行
        char *pt = strstr(buffer, "\r\n");
        int pos = pt - buffer;
        buffer[pos] = '\0';
        parseRequestLine(buffer, info->_fd);
    }
    else if (len == 0)
    {
        // 客户端断开连接
        epoll_ctl(info->_epfd, EPOLL_CTL_DEL, info->_fd, NULL);
        close(info->_fd);
    }
    else
    {
        perror("recv");
    }
    printf("recvHttpRequest tid:%ld\n", info->tid);
    free(info);
    return 0;
}
/*------------------------------------------------------------------------------------------*/

// 解析请求行
int parseRequestLine(const char *line, int cfd)
{
    char method[12];
    char path[1024];
    decodeMsg(path, path);
    // 分割请求头
    sscanf(line, "%[^ ] %[^ ]", method, path);
    // 只解析get请求
    if (strcasecmp(method, "get") != 0)
    {
        // 没找到返回-1
        return -1;
    }
    // 处理客户端请求的静态资源（目录或文件）
    char *file = NULL;
    // 请求的资源是不是根目录
    if (strcmp(path, "/") == 0)
    {
        file = "./";
    }
    else
    {
        file = path + 1;
    }
    // 获取文件属性
    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1)
    {
        // 文件不存在--404
        sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
        sendFile("404.html", cfd);
        return 0;
    }
    // 判断文件类型
    if (S_ISDIR(st.st_mode))
    {
        // 是目录
        sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
        sendDir(cfd, file);
    }
    else
    {
        // 是文件
        sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
        sendFile(file, cfd);
    }

    return 0;
}

/*------------------------------------------------------------------------------------------*/

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
int sendDir(int cfd, const char *dirName)
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
        send(cfd, buf, strlen(buf), 0);
        // 数据发送完缓冲区清零
        memset(buf, 0, sizeof(buf));
        free(namelise[i]);
    }
    sprintf(buf, "</table></body></html>");
    send(cfd, buf, strlen(buf), 0);
    free(namelise);
    return 0;
}
/*------------------------------------------------------------------------------------------*/

// 发送文件
int sendFile(const char *namefile, int cfd)
{
    // 打开文件
    int fd = open(namefile, O_RDONLY);
    assert(fd > 0);
#if 0
    //version 1
    while (1)
    {
        char buffer[1024];
        // 从对应文件描述符中读数据到buffer
        int len = read(fd, buffer, sizeof buffer);
        if (len > 0)
        {
            // 发送数据
            send(cfd, buffer, sizeof buffer, 0);
            usleep(10); // 防止发送太快客户端来不及解析
        }
        //没数据可读了
        else if (len == 0)
        {
            break;
        }
        else
        {
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

/*------------------------------------------------------------------------------------------*/

// 发送响应头（状态行+响应头）
int sendHeadMsg(int cfd, int stat, char *descr, const char *type, int length)
{
    char buffer[4096] = {0};
    // 状态行
    sprintf(buffer, "http/1.1 %d %s\r\n", stat, descr);
    // 响应头
    sprintf(buffer + strlen(buffer), "content-type: %s\r\n", type);
    sprintf(buffer + strlen(buffer), "content-length: %d\r\n\r\n", length);
    send(cfd, buffer, strlen(buffer), 0);
    return 0;
}

/*
<html>
    <head>
        <title>test</title>
    </head>
    <body>
        <table>
            <tr>
                <td></td>
                <td></td>
            </tr>
            <tr>
                <td></td>
                <td></td>
            </tr>
        </table>
    </body>
</html>
*/

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