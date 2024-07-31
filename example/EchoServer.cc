#include "TcpServer.h"
#include <string>
#include "Logger.h"
class EchoServer
{
public:
    EchoServer(EventLoop *loop,
               const InetAddress &addr,
               const std::string &name)
        : server_(loop, addr, name),
          loop_(loop)
    {
    }
    ~EchoServer();

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
    }
    EventLoop *loop_;
    TcpServer server_;
};

int main()
{

    return 0;
}
