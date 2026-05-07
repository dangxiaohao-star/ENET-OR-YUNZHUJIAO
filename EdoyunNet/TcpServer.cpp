#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"

TcpServer::TcpServer(EventLoop *event_loop)
    : event_loop_(event_loop)
    , acceptor_(new Acceptor(event_loop))
{
    acceptor_->SetNewConnectionCallback([this](int fd) {
        TcpConnection::TcpConnPtr conn = this->OnConnect(fd);
        if (conn) {
            this->AddConnection(fd, conn);
            conn->SetDisconnectCallback([this](TcpConnection::TcpConnPtr conn) {
                int fd = conn->GetSocket();
                this->RemoveConnection(fd);
            });
        }
    });
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

    return false;
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
