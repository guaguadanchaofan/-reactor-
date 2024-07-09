#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "TcpServer.h"
int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("./a.out port path");
        return -1;
    }
    // atoi 字符串转整形
    unsigned short port = atoi(argv[1]);
    // 切换工作目录
    chdir(argv[2]);
    // 启动tcp服务
    struct TcpServer *Tcpserver = initTcpServer(port, 1);
    runTcpServer(Tcpserver);
    return 0;
}