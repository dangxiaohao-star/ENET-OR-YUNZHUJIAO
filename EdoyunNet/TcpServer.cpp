#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"

TcpServer::TcpServer(EventLoop *event_loop)
    : event_loop_(event_loop)
    , acceptor_(new Acceptor(event_loop))
{
    std::cout << "[DEBUG] TcpServer构造函数调用" << std::endl;
    std::cout << "[DEBUG] EventLoop地址: " << event_loop_ << std::endl;
    std::cout << "[DEBUG] Acceptor地址: " << acceptor_.get() << std::endl;

    acceptor_->SetNewConnectionCallback([this](int fd) {
        std::cout << "[DEBUG] 新连接回调触发! fd=" << fd << std::endl;

        TcpConnection::TcpConnPtr conn = this->OnConnect(fd);
        if (conn) {
            std::cout << "[SUCCESS] TCP连接创建成功 fd=" << fd << std::endl;
            std::cout << "[DEBUG] 连接对象地址: " << conn.get() << std::endl;
            std::cout << "[DEBUG] 引用计数: " << conn.use_count() << std::endl;
            this->AddConnection(fd, conn);
            conn->SetDisconnectCallback([this](TcpConnection::TcpConnPtr conn) {
                int fd = conn->GetSocket();
                this->RemoveConnection(fd);
            });
        }
        else {
            std::cout << "[ERROR] TCP连接创建失败 fd=" << fd << std::endl;
            std::cout << "[DEBUG] 关闭socket fd=" << fd << std::endl;
        }
    });
    std::cout << "[DEBUG] TcpServer构造完成" << std::endl;
}

TcpServer::~TcpServer()
{
    this->Stop();
}

bool TcpServer::Start(std::string ip, uint16_t port)
{
    this->Stop();
    if (false == is_started_) {
        if (acceptor_->Listen(ip, port) < 0) {
            return false;
        }

        ip_ = ip;
        port_ = port;
        is_started_ = true;
    }

    return true;
}

void TcpServer::Stop()
{
    if (is_started_) {
        for (auto& [fd, conn] : connections_) {
            conn->Disconnect();
        }

        acceptor_->Close();
        is_started_ = false;
    }
}

TcpConnection::TcpConnPtr TcpServer::OnConnect(int sockfd)
{
    return std::make_shared<TcpConnection>(event_loop_->GetTaskScheduler().get(), sockfd);
}

void TcpServer::AddConnection(int sockfd, TcpConnection::TcpConnPtr conn)
{
    connections_.emplace(sockfd, conn);
}

void TcpServer::RemoveConnection(int sockfd)
{
    connections_.erase(sockfd);
}
