#include "TcpConnection.h"
#include <unistd.h> // close 函数所需头文件
// 通过外部传入的sockfd创建Channel,并添加到TaskScheduler对象中进行事件监听和处理
TcpConnection::TcpConnection(TaskScheduler *task_scheduler, int sockfd)
    : task_scheduler_(task_scheduler)
    , r_buffer_(new BufferReader(/* 缺省2048 */))
    , w_buffer_(new BufferWriter(500))
    , channel_(new Channel(sockfd))

{
    is_closed_ = false;
    channel_->SetReadCallback([this]() { this->HandleRead(); });
    channel_->SetWriteCallback([this]() { this->HandleWrite(); });
    channel_->SetErrorCallback([this]() { this->HandleError(); });
    channel_->SetCloseCallback([this]() { this->HandleClose(); });
    
    // 设置套接字属性
    SocketUtil::SetNonBlock(sockfd);
    SocketUtil::SetSendBufSize(sockfd, 100 * 1024); // 这里的SendBuf和我们的r_buffer_的关系是？
    SocketUtil::SetKeepAlive(sockfd);

    channel_->EnableReading();
    task_scheduler_->UpdateChannel(channel_);
}

TcpConnection::~TcpConnection()
{
    int fd = channel_->GetSockfd();
    if (fd > 0) {
        ::close(fd);
    }
}

void TcpConnection::Send(std::shared_ptr<char> data, uint32_t size)
{
    if (is_closed_) return;

    w_buffer_->Append(data, size);
    this->HandleWrite();
}

void TcpConnection::Send(const char *data, uint32_t size)
{
    if (is_closed_) return;

    w_buffer_->Append(data, size);
    this->HandleWrite();
}

void TcpConnection::Disconnect()
{
    if (is_closed_) return;

    this->Close();
}

void TcpConnection::HandleRead()
{
    if (is_closed_) return;

    int ret = r_buffer_->Read(channel_->GetSockfd());
    if (ret < 0) {
        this->Disconnect();
        return;
    }

    if (read_cb_) {
        bool ret = read_cb_(shared_from_this(), *r_buffer_);
        if (false == ret) {
            this->Disconnect();
        }
    }
}

void TcpConnection::HandleWrite()
{
    if (is_closed_) return;

    int ret = 0;
    bool empty = false;
    do {
        ret = w_buffer_->Send(channel_->GetSockfd());
        if (ret < 0) {
            this->Disconnect();
            return;
        }
        empty = w_buffer_->IsEmpty();
    } while (0);

    if (empty) {
        if (channel_->IsWriting()) {
            channel_->DisableWriting();
            task_scheduler_->UpdateChannel(channel_);
        }
    }
    else if (!channel_->IsWriting()) {
        channel_->EnableWriting();
        task_scheduler_->UpdateChannel(channel_);
    }
}

void TcpConnection::HandleError()
{
    this->Close();
}

void TcpConnection::HandleClose()
{
    this->Close();
}

void TcpConnection::Close()
{
    if (is_closed_) return;

    is_closed_ = true;
    task_scheduler_->RemoveChannel(channel_);
    if (close_cb_) {
        close_cb_(shared_from_this());
    }
    if (disconnect_cb_) {
        disconnect_cb_(shared_from_this());
    }
}
