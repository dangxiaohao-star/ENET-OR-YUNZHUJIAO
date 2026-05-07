#include <memory>
#include <string>
#include <unordered_map>
#include "TcpConnection.h"  // 包含了TcpSocket.h

class EventLoop;
class Acceptor;

class TcpServer
{
public:
    TcpServer(EventLoop* event_loop);
    virtual ~TcpServer();

    virtual bool Start(std::string ip, uint16_t port);
    virtual void Stop();

    std::string GetIPAddress() const { return ip_; }
    uint16_t GetPort() const { return port_; }

protected:
    // 新连接发生的时候 创建 TcpConnection 并在 event_loop_ 中添加关心的I/O事件
    virtual TcpConnection::TcpConnPtr OnConnect(int sockfd);

    // 添加新连接对象至 connections_
    virtual void AddConnection(int sockfd, TcpConnection::TcpConnPtr conn);
    // 将连接断开的对象从 connections_中移除
    virtual void RemoveConnection(int sockfd);

private:
    EventLoop* event_loop_ = nullptr;
    std::string ip_ = "";
    uint16_t port_ = 0;
    std::unique_ptr<Acceptor> acceptor_;
    bool is_started_ = false;;
    std::unordered_map<int, TcpConnection::TcpConnPtr> connections_;
};