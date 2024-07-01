
#include "server.c"
int main()
{
    // 初始化用于监听的套接字
    //端口取值范围0-65536
    int lfd = initListenFD(8080);
    // 启动服务器程序
    return 0;
}