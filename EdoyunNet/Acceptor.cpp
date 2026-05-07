#include "Acceptor.h"
#include "EventLoop.h"
#include <unistd.h>     // close函数需要包含这个头文件

Acceptor::Acceptor(EventLoop *loop)
    : event_loop_(loop)
    , tcp_socket_(std::make_shared<TcpSocket>())
{
    std::cout << "[DEBUG] Acceptor create successful" << std::endl;
}

Acceptor::~Acceptor()
{
}

int Acceptor::Listen(std::string ip, uint16_t port)
{
    if (tcp_socket_->GetSocket() > 0) {
        tcp_socket_->Close();
    }
    int fd = tcp_socket_->Create();
    channel_ptr_.reset(new Channel(fd));    // 为什么用reset, 还可以用make_shared吗？因为Channel的构造函数需要一个参数，而make_shared不支持传递参数(?)，所以只能先创建一个空的shared_ptr，然后用reset来重新分配内存并调用构造函数?
    SocketUtil::SetNonBlock(fd);
    SocketUtil::SetReuseAddr(fd);
    SocketUtil::SetReusePort(fd);

    if (!tcp_socket_->Bind(ip, port)) {
        return -1;
    }

    if (!tcp_socket_->Listen(1024)) {   // backlog设置为1024，表示内核中完成连接队列的最大长度为1024
        return -2;
    }

    channel_ptr_->SetReadCallback([this]() { this->OnAccept(); });   // 监听套接字可读事件发生时，调用Acceptor的OnAccept成员函数来处理新连接
    channel_ptr_->EnableReading();    // 监听套接字感兴趣的事件是可读事件，即有新连接到来时触发
    event_loop_->UpdateChannel(channel_ptr_);    // 将监听套接字对应的Channel注册到EventLoop中，开始监听事件
    return 0;
}

void Acceptor::Close()
{
    if (tcp_socket_->GetSocket() > 0) {
        event_loop_->RemoveChannel(channel_ptr_);   // 从EventLoop中移除监听套接字对应的Channel，停止监听事件
        tcp_socket_->Close();                       // 关闭监听套接字
    }
}

void Acceptor::OnAccept()
{
    int fd = tcp_socket_->Accept();    // 接受新连接，返回新连接的文件描述符
    if (fd > 0) {
        if (new_conncb_) {   // 如果用户设置了新连接回调函数，则调用该函数，并传入新连接的文件描述符
            new_conncb_(fd);
        } else {
            ::close(fd);   // 否则直接关闭新连接的文件描述符，避免资源泄漏
        }
    }
}
