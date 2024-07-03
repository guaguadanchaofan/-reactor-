#include "server.c"
int main(int argc, char* argv[])
{
    if(argc<3)
    {
        printf("./a.out port path");
        return -1;
    }
    //atoi 字符串转整形
    unsigned short port = atoi(argv[1]);
    //切换工作目录
    chdir(argv[2]);
    // 初始化用于监听的套接字
    //端口取值范围0-65536
    int lfd = initListenFD(port);

    // 启动服务器程序
    epollRun(lfd);
    return 0;
}