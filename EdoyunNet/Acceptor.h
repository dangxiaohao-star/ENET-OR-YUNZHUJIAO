#include <functional>
#include <memory>
#include "Channel.h"
#include "TcpSocket.h"

class EventLoop;    // 声明EventLoop类，避免循环依赖

typedef std::function<void(int)> NewConnectionCallback;

class Acceptor {
public:
    Acceptor(EventLoop* loop);
    ~Acceptor();

    // C++17 已隐式认为inline
    inline void SetNewConnectionCallback(const NewConnectionCallback& cb) { new_conncb_ = cb;  };
    int Listen(std::string ip, uint16_t port);
    void Close();

private:
    void OnAccept();

private:
    EventLoop* event_loop_ = nullptr;    // 事件循环
    // shared_ptr类型默认初始化会自动为空，显式写出来更清晰
    std::shared_ptr<TcpSocket> tcp_socket_ = nullptr;    // TCP套接字
    ChannelPtr channel_ptr_ = nullptr;    // 监听套接字对应的Channel

    NewConnectionCallback new_conncb_;    // 新连接回调函数
};