#pragma once
#include<netinet/in.h>
#include<string>
class InetAddress
{
public:
    explicit InetAddress(uint16_t port,std::string ip = "localhost");
    explicit InetAddress(const sockaddr_in& addr);
    std::string toIP()const;
    std::string toIpPort()const;
    uint16_t toPort()const;
    const sockaddr_in* getSockAddr()const{return &addr_;}
    ~InetAddress();
private:
    sockaddr_in addr_;
};
