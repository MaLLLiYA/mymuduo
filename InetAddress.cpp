#include <strings.h> // bzero
#include <string.h>  // strlen

#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const
{
    // sockaddr_in addr_;
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const
{
    // ip:port
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    // port
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

uint16_t InetAddress::toPort() const
{

    return (ntohs(addr_.sin_port));
}

// #include <iostream>

// int main()
// {

//     InetAddress addr(8080);
//     std::cout << addr.toIpPort() << std::endl;
//     std::cout << addr.toIp() << std::endl;
//     std::cout << addr.toPort() << std::endl;

//     InetAddress addr2(8080, "172.168.200.2");
//     std::cout << addr2.toIpPort() << std::endl;
//     std::cout << addr2.toIp() << std::endl;
//     std::cout << addr2.toPort() << std::endl;
//     return 0;
// }
