#include "TcpSocket.h"
#include <fcntl.h>          // 重要：fcntl函数需要包含这个头文件
#include <sys/socket.h>     // 重要：setsockopt等函数需要包含这个头文件
#include <arpa/inet.h>      // 重要: sockaddr_in等结构体需要包含这个头文件
#include <unistd.h>         // 重要：close函数需要包含这个头文件

// 以下是Socket工具类相关成员函数实现
void SocketUtil::SetNonBlock(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void SocketUtil::SetBlock(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK);
}

void SocketUtil::SetReuseAddr(int sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on));
}

void SocketUtil::SetReusePort(int sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const void*)&on, sizeof(on));
}

void SocketUtil::SetKeepAlive(int sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const void*)&on, sizeof(on));
}

void SocketUtil::SetSendBufSize(int sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const void*)&size, sizeof(size));
}

void SocketUtil::SetRecvBufSize(int sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const void*)&size, sizeof(size));
}

// 以下是TcpSocket类的成员函数实现
TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

int TcpSocket::Create()
{
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        return -1;
    }
    return sockfd_;
}

// bind
bool TcpSocket::Bind(std::string ip, short port)
{
    if (sockfd_ == -1) return false;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if (::bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {   // ***
        return false;
    }
    return true;
}

// listen
bool TcpSocket::Listen(int backlog)
{
    if (sockfd_ == -1) return false;
    if (listen(sockfd_, backlog) == -1) {
        return false;
    }
    return true;
}

int TcpSocket::Accept()
{
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(addr);
    return ::accept(sockfd_, (struct sockaddr*)&addr, &addrlen);
}

void TcpSocket::Close()
{
    if (sockfd_ != -1) {
        ::close(sockfd_);
        sockfd_ = -1;
    }
}

void TcpSocket::ShutDownWrite()
{
    if (sockfd_ != -1) {
        shutdown(sockfd_, SHUT_WR);
        sockfd_ = -1;
    }
}
