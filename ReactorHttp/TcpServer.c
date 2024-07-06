#include "TcpServer.h"
#include "ThreadPool.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include"TcpConnection.h"
// 初始化
struct TcpServer *initTcpServer(unsigned short port, int threadNum)
{
    struct TcpServer *tcp = (struct TcpServer *)malloc(sizeof(struct TcpServer));
    // 初始化主反应堆
    tcp->_mainloop = initEventLoop();
    // 初始化线程池
    tcp->_threadNum = threadNum;
    tcp->_pool = initThreadPool(tcp->_mainloop, threadNum);
    tcp->_listener = initListener(port);
    return tcp;
}

// 初始化listener
struct Listener *initListener(unsigned short port)
{

    struct Listener *listener = (struct Listener *)malloc(sizeof(struct Listener));
    // 1.创建监听套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        return -1;
    }

    // 2.设置端口复用
    int opt = 1;
    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1)
    {
        perror("setsockopt");
        return -1;
    }

    // 3.绑定端口号和IP地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    // 注意要采用大端（低地址储存高位） INADDR_ANY表示可以用任何地址
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
    listener->lfd = lfd;
    listener->port = port;
    return listener;
}

// 同意连接
int acceptConnection(void *arg)
{
    struct TcpServer *server = (struct TcpServer *)arg;
    // 获取通信文件描述符
    int cfd = accept(server->_listener->lfd, NULL, NULL); // 第二参数用于保存和客户端建立连接的ip跟端口 第三个参数用于描述第二个参数结构体的大小
    //从线程池里面取出一个子线程的反应堆实例 处理这个cfd
    struct EventLoop* EventLoop=takeWorkEventLoop(server->_pool);
    //将cfd放到TcpConnection中处理
    initTcpConnection(EventLoop,cfd);

    return 0;
}

// 启动tcpserver
void runListener(struct TcpServer *tcpserver)
{
    // 启动线程池
    runThreadPool(tcpserver->_pool);
    // 添加检测任务
    struct Channel *channel = initchannel(tcpserver->_listener->lfd, readevent, NULL, acceptConnection, tcpserver);
    AddTaskEventLoop(tcpserver->_mainloop, channel, ADD);
}