#include "server.h"

// 初始化监听
int initListenFD(uint16_t port)
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
    // 注意要采用大端（低地址储存高位） INADDR_ANY表示0不区分大小端
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
    // 数组多大无所谓
    

    // 3.检测（时间未知需要while循环检测）
    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);
    while (1)
    {
        // 事件等待，将就绪的事件数返回
        int num = epoll_wait(epfd, &evs, size, -1);
        // 遍历就绪的事件
        for (int i = 0; i < num; i++)
        {
            int fd = evs[i].data.fd;
            // 如果当前fd是用于监听的文件描述符
            if (fd == lfd)
            {
                // 建立新连接（事件已经发生 不会阻塞）
                acceptClient(lfd,epfd);
            }
            // 用于通信
            else
            {
                // 主要是接受对端数据（读数据）
            }
        }
    }
}

/*-----------------------------------------------------------------------------------------*/

//和客户端建立连接
int acceptClient(int lfd, int epfd)
{
    //1.建立连接
    //第23个参数为传出参数用于保存客户端的ip跟port信息
    int cfd=accept(lfd,NULL,NULL);
    if(cfd==-1)
    {
        perror("accept");
        return -1;
    }


    //2.设置非阻塞
    /*复制一个现有的描述符(cmd=F_DUPFD)
    获得/设置文件描述符标记(cmd=F_GETFD或F_SETFD)
    获得/设置文件状态标记(cmd=F_GETFL或F_SETFL)
    获得/设置异步I/O所有权(cmd=F_GETOWN或F_SETOWN)
    获得/设置记录锁(cmd=F_GETLK,F_SETLK或F_SETLKW)*/
    int flag=fcntl(cfd,F_GETFL);
    flag|=O_NONBLOCK;
    //ET(边缘触发 需要设置非阻塞) LT(水平触发)
    int ret = fcntl(cfd,F_SETFL,flag);

    //3.cfd添加到epoll中
    //创建epoll事件结构体
    struct epoll_event ev;
    ev.data.fd=cfd;
    //检测读事件就绪(读事件|边缘触发事件)
    ev.events=EPOLLIN|EPOLLET;
    int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
    if(ret==-1)
    {
        perror("epoll_ctl");
        return -1;
    }
}
